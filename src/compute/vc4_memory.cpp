#include "piai/compute/vc4_memory.hpp"

#include <cerrno>
#include <cstdint>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace piai::compute {
namespace {
constexpr unsigned kMailboxMajor = 100;
constexpr unsigned kMailboxProperty = _IOWR(kMailboxMajor, 0, char*);
constexpr uint32_t kTagAlloc = 0x0003000c;
constexpr uint32_t kTagLock = 0x0003000d;
constexpr uint32_t kTagUnlock = 0x0003000e;
constexpr uint32_t kTagFree = 0x0003000f;
constexpr uint32_t kFlagDirect = 1u << 2;
constexpr uint32_t kFlagCoherent = 2u << 2;
constexpr uint32_t kFlagPermanent = 1u << 6;
constexpr uint32_t kBusMask = 0xC0000000u;

struct alignas(16) Msg {
    uint32_t size;
    uint32_t code;
    uint32_t tag;
    uint32_t value_bytes;
    uint32_t value_len;
    uint32_t value[3];
    uint32_t end;
};

bool property(int fd, Msg& m) {
    m.size = sizeof(m);
    m.code = 0;
    return ioctl(fd, kMailboxProperty, &m) >= 0 && (m.code & 0x80000000u) != 0;
}

uint32_t call1(int fd, uint32_t tag, uint32_t value) {
    Msg m{};
    m.tag = tag;
    m.value_bytes = 4;
    m.value_len = 4;
    m.value[0] = value;
    m.end = 0;
    return property(fd, m) ? m.value[0] : 0;
}

} // namespace

VC4Memory::~VC4Memory() { release(); }

bool VC4Memory::allocate(size_t bytes, size_t alignment, bool coherent) {
    release();
    if (bytes == 0 || alignment == 0 || (alignment & (alignment - 1)) != 0 || bytes > 0xffffffffu)
        return false;

    mailbox_ = ::open("/dev/vcio", O_RDWR | O_CLOEXEC);
    if (mailbox_ < 0) return false;

    Msg m{};
    m.tag = kTagAlloc;
    m.value_bytes = 12;
    m.value_len = 12;
    m.value[0] = static_cast<uint32_t>((bytes + 4095u) & ~size_t(4095u));
    m.value[1] = static_cast<uint32_t>(alignment);
    // Direct/coherent are documented VideoCore aliases. Permanent-lock avoids
    // the buffer moving while its bus address is handed to QPU DMA.
    m.value[2] = (coherent ? kFlagCoherent : kFlagDirect) | kFlagPermanent;
    if (!property(mailbox_, m) || m.value[0] == 0) {
        ::close(mailbox_); mailbox_ = -1; return false;
    }
    handle_ = m.value[0];
    bus_ = call1(mailbox_, kTagLock, handle_);
    if (bus_ == 0) { release(); return false; }

    const uintptr_t phys = static_cast<uintptr_t>(bus_ & ~kBusMask);
    const size_t mapped = static_cast<size_t>((m.value[0] ? ((bytes + 4095u) & ~size_t(4095u)) : bytes));
    cpu_ = mmap(nullptr, mapped, PROT_READ | PROT_WRITE, MAP_SHARED, ::open("/dev/mem", O_RDWR | O_SYNC), phys);
    if (cpu_ == MAP_FAILED) cpu_ = nullptr;
    if (!cpu_) { release(); return false; }
    size_ = mapped;
    return true;
}

void VC4Memory::release() {
    if (cpu_) {
        munmap(cpu_, size_);
        cpu_ = nullptr;
    }
    if (mailbox_ >= 0 && handle_) {
        call1(mailbox_, kTagUnlock, handle_);
        call1(mailbox_, kTagFree, handle_);
    }
    handle_ = 0;
    bus_ = 0;
    size_ = 0;
    if (mailbox_ >= 0) { ::close(mailbox_); mailbox_ = -1; }
}

bool vc4_memory_available() {
    const int fd = ::open("/dev/vcio", O_RDWR | O_CLOEXEC);
    if (fd < 0) return false;
    ::close(fd);
    return ::access("/dev/mem", R_OK | W_OK) == 0;
}

} // namespace piai::compute
