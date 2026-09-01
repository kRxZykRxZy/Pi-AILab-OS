#include "gguf.hpp"
#include "quant.hpp"
#include "transformer.hpp"
#include "api.hpp"
#include <cstdio>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <chrono>
int main(int argc,char**argv){
 piai::api::Server server;
 if(!server.listen(5453)){std::fprintf(stderr,"piai: unable to bind API port 5453\n");return 10;}
 std::printf("Pi-AI Lab runtime v0.1\nAPI listening on port 5453\n");
 if(argc>=2){piai::gguf::Model m;if(!m.open(argv[1])){std::fprintf(stderr,"piai: invalid or unsupported GGUF: %s\n",argv[1]);return 2;}std::printf("GGUF v%u, tensors=%zu, data_offset=%llu\n",m.version(),m.tensors().size(),(unsigned long long)m.data_offset());}
 server.serve_forever(); return 0;
}
