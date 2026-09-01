#include "piai/runtime.h"
#include "piai/config.h"
#include "piai/http.h"
#include <cstdio>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>
namespace { std::atomic<bool> stop{false}; void sig(int){stop=true;} }
int main(int argc,char** argv){std::signal(SIGINT,sig);std::signal(SIGTERM,sig); auto c=piai::load_config(argc>1?argv[1]:"/etc/piai/piai.conf"); piai::Runtime r; if(!r.start()) return 1; std::thread api([&]{piai::http_run(r,c.bind.c_str(),c.port);}); std::printf("Pi-AI Lab Runtime v0.1\n"); std::fflush(stdout); while(!stop) std::this_thread::sleep_for(std::chrono::seconds(1)); r.stop(); api.join(); return 0; }
