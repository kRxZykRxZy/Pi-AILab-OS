#pragma once
#include <cstddef>
#include <cstdint>
#include <string>

namespace piai::compute {

struct VC4RuntimeInfo {
    bool mailbox = false;
    bool memory = false;
    bool qpu = false;
    unsigned qpus = 12;
    unsigned timeout_ms = 1000;
    std::string reason;
};

VC4RuntimeInfo vc4_runtime_info();
bool vc4_runtime_self_test();

// Execute a pre-assembled VC4 program. uniforms are copied into GPU-visible
// memory and one control pair is emitted per QPU thread.
bool vc4_execute_program(const uint8_t* code, size_t code_bytes,
                         const uint32_t* uniforms, size_t uniform_words,
                         unsigned qpus, unsigned timeout_ms);

} // namespace piai::compute
