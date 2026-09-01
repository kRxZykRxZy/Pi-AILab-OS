#pragma once
#include <algorithm>
#include <atomic>
#include <thread>

namespace piai::cpu {

inline unsigned detected_cpus() {
    unsigned n = std::thread::hardware_concurrency();
    return n ? n : 1u;
}

// Default: use 75% of available logical CPUs, rounded down, minimum 1.
inline unsigned default_ai_threads() {
    const unsigned n = detected_cpus();
    return std::max(1u, (n * 3u) / 4u);
}

inline std::atomic<unsigned>& ai_threads_setting() {
    static std::atomic<unsigned> value{default_ai_threads()};
    return value;
}

inline unsigned ai_threads() {
    return std::max(1u, std::min(detected_cpus(), ai_threads_setting().load(std::memory_order_relaxed)));
}

inline bool set_ai_threads(unsigned value) {
    if (value < 1 || value > detected_cpus()) return false;
    ai_threads_setting().store(value, std::memory_order_relaxed);
    return true;
}

inline unsigned default_cpu_percent() { return 75u; }

} // namespace piai::cpu
