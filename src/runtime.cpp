#include "piai/runtime.h"
#include <chrono>
namespace piai { static uint64_t now(){return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();} bool Runtime::start(){if(running_.exchange(true)) return false; started_ms_=now(); return true;} void Runtime::stop(){running_=false;} bool Runtime::running() const noexcept{return running_.load();} uint64_t Runtime::uptime_ms() const noexcept{return running_?now()-started_ms_:0;} }
