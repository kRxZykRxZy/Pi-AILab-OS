#include <string>
#include <thread>
namespace piai { std::string system_arch(){
#if defined(__aarch64__) return "armv8";
#elif defined(__arm__) return "arm32";
#elif defined(__x86_64__) return "x86_64";
#elif defined(__i386__) return "x86";
#else return "unknown";
#endif
} unsigned hardware_threads(){return std::thread::hardware_concurrency();} }
