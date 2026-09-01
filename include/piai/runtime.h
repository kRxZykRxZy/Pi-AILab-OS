#pragma once
#include <atomic>
#include <cstdint>
namespace piai { class Runtime { public: bool start(); void stop(); bool running() const noexcept; uint64_t uptime_ms() const noexcept; private: std::atomic<bool> running_{false}; uint64_t started_ms_{0}; }; }
