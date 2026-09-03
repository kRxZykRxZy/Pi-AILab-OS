#include "piai/compute/vc4.hpp"
#include "gguf.hpp"
#include "piai/inference/types.hpp"

#include <cstdlib>
#include <cstring>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#if defined(PIAI_HAVE_OPENCL)
#include <CL/cl.h>
#endif

namespace piai::compute {

#if defined(PIAI_HAVE_OPENCL)
namespace {

static const char* kMatvec = R"CLC(
uint f32bits(ushort h) {
    uint sign = ((uint)(h & (ushort)0x8000)) << 16;
    uint exp  = ((uint)(h >> 10)) & 31u;
    uint mant = ((uint)h) & 1023u;
    if (exp == 0u) {
        if (mant == 0u) return sign;
        float f = (float)mant * 0.000000059604644775390625f;
        return as_uint((sign != 0u) ? -f : f);
    }
    if (exp == 31u) {
        return sign | 0x7f800000u | (mant << 13);
    }
    return sign | ((exp + 112u) << 23) | (mant << 13);
}

__kernel void matvec_f16(__global const ushort* w,
                         __global const float* x,
                         __global float* y,
                         uint cols) {
    uint row = get_global_id(0);
    float acc = 0.0f;
    uint base = row * cols;
    for (uint j = 0; j < cols; ++j)
        acc = fma(as_float(f32bits(w[base + j])), x[j], acc);
    y[row] = acc;
}

__kernel void matvec_f32(__global const float* w,
                         __global const float* x,
                         __global float* y,
                         uint cols) {
    uint row = get_global_id(0);
    float acc = 0.0f;
    uint base = row * cols;
    for (uint j = 0; j < cols; ++j)
        acc = fma(w[base + j], x[j], acc);
    y[row] = acc;
}
)CLC";

struct BufferEntry {
    cl_mem buffer = nullptr;
    size_t bytes = 0;
};

class VC4Runtime {
    cl_platform_id platform_ = nullptr;
    cl_device_id device_ = nullptr;
    cl_context context_ = nullptr;
    cl_command_queue queue_ = nullptr;
    cl_program program_ = nullptr;
    cl_kernel f16_ = nullptr;
    cl_kernel f32_ = nullptr;
    cl_mem input_ = nullptr;
    cl_mem output_ = nullptr;
    size_t input_bytes_ = 0;
    size_t output_bytes_ = 0;
    size_t max_group_ = 1;
    std::unordered_map<const void*, BufferEntry> weights_;
    std::mutex mu_;
    bool ready_ = false;

    static bool ok(cl_int e) { return e == CL_SUCCESS; }

    void reset() {
        for (auto& kv : weights_) if (kv.second.buffer) clReleaseMemObject(kv.second.buffer);
        weights_.clear();
        if (input_) clReleaseMemObject(input_);
        if (output_) clReleaseMemObject(output_);
        if (f16_) clReleaseKernel(f16_);
        if (f32_) clReleaseKernel(f32_);
        if (program_) clReleaseProgram(program_);
        if (queue_) clReleaseCommandQueue(queue_);
        if (context_) clReleaseContext(context_);
        input_ = output_ = nullptr;
        f16_ = f32_ = nullptr;
        program_ = nullptr;
        queue_ = nullptr;
        context_ = nullptr;
        platform_ = nullptr;
        device_ = nullptr;
        ready_ = false;
    }

    bool init_locked() {
        if (ready_) return true;
        cl_uint np = 0;
        if (!ok(clGetPlatformIDs(0, nullptr, &np)) || np == 0) return false;
        std::vector<cl_platform_id> platforms(np);
        if (!ok(clGetPlatformIDs(np, platforms.data(), nullptr))) return false;

        for (auto p : platforms) {
            char name[256] = {};
            char vendor[256] = {};
            clGetPlatformInfo(p, CL_PLATFORM_NAME, sizeof(name), name, nullptr);
            clGetPlatformInfo(p, CL_PLATFORM_VENDOR, sizeof(vendor), vendor, nullptr);
            cl_uint nd = 0;
            if (!ok(clGetDeviceIDs(p, CL_DEVICE_TYPE_GPU, 0, nullptr, &nd)) || nd == 0) continue;
            std::vector<cl_device_id> devices(nd);
            if (!ok(clGetDeviceIDs(p, CL_DEVICE_TYPE_GPU, nd, devices.data(), nullptr))) continue;
            for (auto d : devices) {
                char dn[256] = {};
                clGetDeviceInfo(d, CL_DEVICE_NAME, sizeof(dn), dn, nullptr);
                std::string s = std::string(name) + " " + vendor + " " + dn;
                if (s.find("VideoCore IV") == std::string::npos &&
                    s.find("VideoCore") == std::string::npos) continue;
                platform_ = p;
                device_ = d;
                break;
            }
            if (device_) break;
        }
        if (!device_) return false;

        cl_int e = CL_SUCCESS;
        context_ = clCreateContext(nullptr, 1, &device_, nullptr, nullptr, &e);
        if (!context_ || !ok(e)) { reset(); return false; }
        queue_ = clCreateCommandQueue(context_, device_, 0, &e);
        if (!queue_ || !ok(e)) { reset(); return false; }
        program_ = clCreateProgramWithSource(context_, 1, &kMatvec, nullptr, &e);
        if (!program_ || !ok(e)) { reset(); return false; }
        e = clBuildProgram(program_, 1, &device_, "-cl-fast-relaxed-math -cl-mad-enable", nullptr, nullptr);
        if (!ok(e)) { reset(); return false; }
        f16_ = clCreateKernel(program_, "matvec_f16", &e);
        if (!f16_ || !ok(e)) { reset(); return false; }
        f32_ = clCreateKernel(program_, "matvec_f32", &e);
        if (!f32_ || !ok(e)) { reset(); return false; }
        size_t wg = 1;
        clGetDeviceInfo(device_, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(wg), &wg, nullptr);
        max_group_ = wg ? wg : 1;
        ready_ = true;
        return true;
    }

    cl_mem weight_buffer(const inference::TensorBinding& w, size_t bytes) {
        auto it = weights_.find(w.data);
        if (it != weights_.end() && it->second.bytes == bytes) return it->second.buffer;
        if (it != weights_.end()) {
            if (it->second.buffer) clReleaseMemObject(it->second.buffer);
            weights_.erase(it);
        }
        cl_int e = CL_SUCCESS;
        cl_mem b = clCreateBuffer(context_, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                                  bytes, const_cast<uint8_t*>(w.data), &e);
        if (!b || !ok(e)) return nullptr;
        weights_[w.data] = {b, bytes};
        return b;
    }

    bool ensure_io(size_t xbytes, size_t ybytes) {
        cl_int e = CL_SUCCESS;
        if (input_bytes_ < xbytes) {
            if (input_) clReleaseMemObject(input_);
            input_ = clCreateBuffer(context_, CL_MEM_READ_ONLY, xbytes, nullptr, &e);
            if (!input_ || !ok(e)) { input_ = nullptr; input_bytes_ = 0; return false; }
            input_bytes_ = xbytes;
        }
        if (output_bytes_ < ybytes) {
            if (output_) clReleaseMemObject(output_);
            output_ = clCreateBuffer(context_, CL_MEM_WRITE_ONLY, ybytes, nullptr, &e);
            if (!output_ || !ok(e)) { output_ = nullptr; output_bytes_ = 0; return false; }
            output_bytes_ = ybytes;
        }
        return true;
    }

public:
    ~VC4Runtime() { std::lock_guard<std::mutex> lock(mu_); reset(); }

    bool available() {
        std::lock_guard<std::mutex> lock(mu_);
        return init_locked();
    }

    bool matvec(const inference::TensorBinding& w, const float* x, float* y, size_t rows, size_t cols) {
        if (!w.tensor || !w.data || !x || !y || !rows || !cols) return false;
        if (w.tensor->type != gguf::TensorType::F16 && w.tensor->type != gguf::TensorType::F32) return false;
        if (rows > 0xffffffffu || cols > 0xffffffffu) return false;
        const size_t element = w.tensor->type == gguf::TensorType::F16 ? 2u : 4u;
        if (cols > SIZE_MAX / element || rows > SIZE_MAX / (cols * element)) return false;
        const size_t bytes = rows * cols * element;
        const size_t xbytes = cols * sizeof(float);
        const size_t ybytes = rows * sizeof(float);

        std::lock_guard<std::mutex> lock(mu_);
        if (!init_locked()) return false;
        cl_mem wb = weight_buffer(w, bytes);
        if (!wb || !ensure_io(xbytes, ybytes)) return false;
        cl_kernel k = w.tensor->type == gguf::TensorType::F16 ? f16_ : f32_;
        cl_int e = clEnqueueWriteBuffer(queue_, input_, CL_FALSE, 0, xbytes, x, 0, nullptr, nullptr);
        if (!ok(e)) return false;
        e  = clSetKernelArg(k, 0, sizeof(cl_mem), &wb);
        e |= clSetKernelArg(k, 1, sizeof(cl_mem), &input_);
        e |= clSetKernelArg(k, 2, sizeof(cl_mem), &output_);
        cl_uint c = (cl_uint)cols;
        e |= clSetKernelArg(k, 3, sizeof(c), &c);
        if (!ok(e)) return false;

        // One work-item per output row. VC4CL maps this workload across the
        // VideoCore IV QPU array; a single enqueue keeps all QPUs available.
        size_t local = max_group_;
        if (local > 64) local = 64;
        while (local > rows && local > 1) local >>= 1;
        size_t global = ((rows + local - 1) / local) * local;
        e = clEnqueueNDRangeKernel(queue_, k, 1, nullptr, &global, &local, 0, nullptr, nullptr);
        if (!ok(e)) return false;
        e = clEnqueueReadBuffer(queue_, output_, CL_TRUE, 0, ybytes, y, 0, nullptr, nullptr);
        return ok(e);
    }
};

static VC4Runtime& runtime() { static VC4Runtime r; return r; }

}
#endif

bool vc4_available() {
#if defined(PIAI_HAVE_OPENCL)
    const char* force = std::getenv("PIAI_VC4_GPU");
    if (force && std::strcmp(force, "0") == 0) return false;
    return runtime().available();
#else
    return false;
#endif
}

bool vc4_matvec(const inference::TensorBinding& weights, const float* x, float* y,
                size_t rows, size_t cols) {
#if defined(PIAI_HAVE_OPENCL)
    const char* force = std::getenv("PIAI_VC4_GPU");
    if (force && std::strcmp(force, "0") == 0) return false;
    if (force && std::strcmp(force, "1") == 0)
        return runtime().matvec(weights, x, y, rows, cols);
    // Auto mode is deliberately conservative: only a detected VideoCore IV
    // device is eligible, and every unsupported tensor falls back to CPU.
    return runtime().matvec(weights, x, y, rows, cols);
#else
    (void)weights; (void)x; (void)y; (void)rows; (void)cols;
    return false;
#endif
}

}
