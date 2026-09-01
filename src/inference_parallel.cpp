#include "inference.hpp"
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <thread>

namespace piai::inference {

static float parallel_hf(uint16_t h) {
    uint32_t s=h>>15,e=(h>>10)&31,f=h&1023;
    if(!e) return (s?-1.f:1.f)*std::ldexp((float)f,-24);
    if(e==31) return f?NAN:(s?-INFINITY:INFINITY);
    return (s?-1.f:1.f)*std::ldexp(1.f+f/1024.f,(int)e-15);
}

static float parallel_elem(const TensorBinding&t,size_t i) {
    if(!t.tensor || !t.data) return 0.f;
    const auto ty=t.tensor->type;
    if(ty==gguf::TensorType::F32) {
        float x; std::memcpy(&x,t.data+i*4,4); return x;
    }
    if(ty==gguf::TensorType::F16) {
        uint16_t x; std::memcpy(&x,t.data+i*2,2); return parallel_hf(x);
    }
    const size_t b=i/32,j=i%32;
    const size_t bs=(ty==gguf::TensorType::Q8_0?34:(ty==gguf::TensorType::Q4_0||ty==gguf::TensorType::Q4_1?18:22));
    const uint8_t*p=t.data+b*bs;
    uint16_t dh; std::memcpy(&dh,p,2);
    float d=parallel_hf(dh);
    if(ty==gguf::TensorType::Q8_0) return d*(int8_t)p[2+j];
    if(ty==gguf::TensorType::Q4_0||ty==gguf::TensorType::Q4_1) {
        uint16_t mh=0;
        if(ty==gguf::TensorType::Q4_1) std::memcpy(&mh,p+2,2);
        const uint8_t*q=p+(ty==gguf::TensorType::Q4_1?4:2);
        int z=(j&1)?q[j/2]>>4:q[j/2]&15;
        return ty==gguf::TensorType::Q4_1?d*z+parallel_hf(mh):d*(z-8);
    }
    uint32_t qh; std::memcpy(&qh,p+2,4);
    const uint8_t*q=p+6;
    int z=(j&1)?q[j/2]>>4:q[j/2]&15;
    z|=((qh>>j)&1)<<4;
    if(ty==gguf::TensorType::Q5_1) {
        uint16_t mh; std::memcpy(&mh,p+2,2);
        return d*z+parallel_hf(mh);
    }
    return d*(z-16);
}

// Matrix-vector products are the dominant inference workload. Split rows over
// every logical CPU reported by the OS so Pi 2/3/4/5 and x86/x64 all scale
// automatically without hard-coding a core count.
static void piai_parallel_mv(const TensorBinding&w,const float*x,float*y,size_t r,size_t c) {
    if(r==0) return;
    unsigned hc=std::thread::hardware_concurrency();
    if(hc==0) hc=1;
    const size_t threads=std::min<size_t>(hc,r);
    if(threads<=1) {
        for(size_t i=0;i<r;i++) {
            double s=0;
            for(size_t j=0;j<c;j++) s+=(double)parallel_elem(w,i*c+j)*x[j];
            y[i]=(float)s;
        }
        return;
    }
    std::vector<std::thread> workers;
    workers.reserve(threads);
    for(size_t t=0;t<threads;t++) {
        const size_t begin=(r*t)/threads;
        const size_t end=(r*(t+1))/threads;
        workers.emplace_back([&,begin,end] {
            for(size_t i=begin;i<end;i++) {
                double s=0;
                for(size_t j=0;j<c;j++) s+=(double)parallel_elem(w,i*c+j)*x[j];
                y[i]=(float)s;
            }
        });
    }
    for(auto&worker:workers) worker.join();
}

}

// Compile the existing complete inference implementation with its matvec
// primitive redirected to the all-CPU implementation above.
#define mv piai_parallel_mv
#include "inference_v4.cpp"
