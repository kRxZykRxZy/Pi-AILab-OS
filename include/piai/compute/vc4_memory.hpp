#pragma once
#include <cstddef>
#include <cstdint>

namespace piai::compute {

// GPU-visible contiguous memory allocated through the VideoCore firmware
// mailbox. The returned bus address is for QPU DMA/control blocks; cpu_ptr()
// is the corresponding ARM mapping.
class VC4Memory {
    int mailbox_ = -1;
    uint32_t handle_ = 0;
    uint32_t bus_ = 0;
    void* cpu_ = nullptr;
    size_t size_ = 0;

public:
    VC4Memory() = default;
    ~VC4Memory();
    VC4Memory(const VC4Memory&) = delete;
    VC4Memory& operator=(const VC4Memory&) = delete;

    bool allocate(size_t bytes, size_t alignment = 4096, bool coherent = true);
    void release();
    void* cpu_ptr() const { return cpu_; }
    uint32_t bus_address() const { return bus_; }
    size_t size() const { return size_; }
    explicit operator bool() const { return cpu_ != nullptr && bus_ != 0; }
};

bool vc4_memory_available();

} // namespace piai::compute
