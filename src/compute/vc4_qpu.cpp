#include "piai/compute/vc4_qpu.hpp"
#include "gguf.hpp"
#include "piai/inference/types.hpp"

#include <cerrno>
#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace piai::compute {
namespace {

// Linux mailbox property interface. QPU execution is the firmware interface
// historically used by VC4 userland programs such as hello_fft.
constexpr unsigned kMailboxMajor = 100;
constexpr unsigned kMailboxProperty = _IOWR(kMailboxMajor, 0, char*);
constexpr uint32_t kTagQpuEnable = 0x00030012;
constexpr uint32_t kTagExecuteQpu = 0x00030011;

class MailboxQpu {
    int fd_ = -1;
    bool enabled_ = false;

    bool property(uint32_t* words, size_t count) {
        if (fd_ < 0 || !words || count < 3) return false;
        words[0] = static_cast<uint32_t>(count * sizeof(uint32_t));
        words[1] = 0;
        return ioctl(fd_, kMailboxProperty, words) >= 0;
    }

public:
    ~MailboxQpu() {
        if (enabled_) enable(false);
        if (fd_ >= 0) close(fd_);
    }

    bool open() {
        if (fd_ >= 0) return true;
        fd_ = ::open("/dev/vcio", O_RDWR | O_CLOEXEC);
        return fd_ >= 0;
    }

    bool enable(bool on) {
        uint32_t msg[8]{};
        msg[2] = kTagQpuEnable;
        msg[3] = 4;
        msg[4] = 4;
        msg[5] = on ? 1u : 0u;
        msg[6] = 0;
        if (!property(msg, 7) || msg[5] != 0) return false;
        enabled_ = on;
        return true;
    }

    // control is the VC address of an array of {uniforms, code} pairs.
    bool execute(unsigned qpus, uint32_t control, unsigned noflush, unsigned timeout_ms) {
        uint32_t msg[10]{};
        msg[2] = kTagExecuteQpu;
        msg[3] = 16;
        msg[4] = 16;
        msg[5] = qpus;
        msg[6] = control;
        msg[7] = noflush;
        msg[8] = timeout_ms;
        msg[9] = 0;
        return property(msg, 10) && msg[5] == 0;
    }
};

} // namespace

bool vc4_qpu_available() {
#if defined(PIAI_ENABLE_VC4_QPU)
    MailboxQpu q;
    return q.open();
#else
    return false;
#endif
}

bool vc4_qpu_matvec(const inference::TensorBinding& weights,
                    const float* x, float* y,
                    size_t rows, size_t cols) {
    // The actual GPU-visible allocator and kernel launch are kept behind this
    // boundary. Never guess a CPU virtual address is a VC bus address: doing
    // so can corrupt arbitrary memory on a Pi. Until the VCSM/CMA staging
    // layer is active, report unsupported and let the tested CPU path run.
    (void)weights;
    (void)x;
    (void)y;
    (void)rows;
    (void)cols;
#if defined(PIAI_ENABLE_VC4_QPU)
    MailboxQpu q;
    if (!q.open()) return false;
    // QPU_ENABLE/EXECUTE_QPU are intentionally not invoked without a
    // GPU-visible allocation and a validated kernel control block.
    return false;
#else
    return false;
#endif
}

} // namespace piai::compute
