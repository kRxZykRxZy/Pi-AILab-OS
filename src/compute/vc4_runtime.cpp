#include "piai/compute/vc4_runtime.hpp"
#include "piai/compute/vc4_memory.hpp"

#include <cstdint>
#include <cstring>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace piai::compute {
namespace {
constexpr unsigned kMailboxMajor = 100;
constexpr unsigned kMailboxProperty = _IOWR(kMailboxMajor, 0, char*);
constexpr uint32_t kTagQpuEnable = 0x00030012;
constexpr uint32_t kTagExecuteQpu = 0x00030011;
constexpr unsigned kMaxQpus = 12;

class Mailbox {
    int fd_ = -1;
public:
    ~Mailbox() { if (fd_ >= 0) ::close(fd_); }
    bool open() { if (fd_ >= 0) return true; fd_ = ::open("/dev/vcio", O_RDWR | O_CLOEXEC); return fd_ >= 0; }
    bool property(uint32_t* words, size_t count) {
        if (fd_ < 0 || !words || count < 4) return false;
        words[0] = static_cast<uint32_t>(count * sizeof(uint32_t));
        words[1] = 0;
        if (ioctl(fd_, kMailboxProperty, words) < 0) return false;
        return (words[1] & 0x80000000u) != 0;
    }
    bool enable(bool on) {
        uint32_t msg[8]{};
        msg[2] = kTagQpuEnable;
        msg[3] = 4;
        msg[4] = 4;
        msg[5] = on ? 1u : 0u;
        return property(msg, 8) && msg[5] == 0;
    }
    bool execute(unsigned qpus, uint32_t control, unsigned timeout_ms) {
        if (qpus == 0 || qpus > kMaxQpus || control == 0) return false;
        uint32_t msg[10]{};
        msg[2] = kTagExecuteQpu;
        msg[3] = 16;
        msg[4] = 16;
        msg[5] = qpus;
        msg[6] = control;
        msg[7] = 0;
        msg[8] = timeout_ms;
        return property(msg, 10) && msg[5] == 0;
    }
};

} // namespace

VC4RuntimeInfo vc4_runtime_info() {
    VC4RuntimeInfo out;
    Mailbox mb;
    out.mailbox = mb.open();
    out.memory = vc4_memory_available();
    out.qpu = out.mailbox && out.memory;
    if (!out.mailbox) out.reason = "/dev/vcio unavailable";
    else if (!out.memory) out.reason = "GPU-visible /dev/mem mapping unavailable";
    else out.reason = "VC4 mailbox and memory interfaces available";
    return out;
}

bool vc4_execute_program(const uint8_t* code, size_t code_bytes,
                         const uint32_t* uniforms, size_t uniform_words_per_thread,
                         unsigned qpus, unsigned timeout_ms) {
    if (!code || code_bytes == 0 || !uniforms || uniform_words_per_thread == 0 ||
        qpus == 0 || qpus > kMaxQpus) return false;

    Mailbox mb;
    if (!mb.open() || !mb.enable(true)) return false;

    const size_t code_size = (code_bytes + 4095u) & ~size_t(4095u);
    const size_t uniform_bytes = uniform_words_per_thread * sizeof(uint32_t);
    const size_t uniform_size = (uniform_bytes * qpus + 4095u) & ~size_t(4095u);
    const size_t control_size = qpus * 2u * sizeof(uint32_t);
    VC4Memory code_mem, uniform_mem, control_mem;
    if (!code_mem.allocate(code_size, 4096, true) ||
        !uniform_mem.allocate(uniform_size, 4096, true) ||
        !control_mem.allocate(control_size, 4096, true)) return false;

    std::memcpy(code_mem.cpu_ptr(), code, code_bytes);
    std::memcpy(uniform_mem.cpu_ptr(), uniforms, uniform_bytes * qpus);
    auto* control = static_cast<uint32_t*>(control_mem.cpu_ptr());
    const uint32_t uniform_bus = uniform_mem.bus_address();
    const uint32_t code_bus = code_mem.bus_address();
    for (unsigned i = 0; i < qpus; ++i) {
        control[i * 2] = uniform_bus + static_cast<uint32_t>(i * uniform_bytes);
        control[i * 2 + 1] = code_bus;
    }
    return mb.execute(qpus, control_mem.bus_address(), timeout_ms ? timeout_ms : 1000);
}

bool vc4_runtime_self_test() {
    return vc4_runtime_info().qpu;
}

} // namespace piai::compute
