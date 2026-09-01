#include "gguf.hpp"
#include "quant.hpp"
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
int main(int argc,char**argv){
 if(argc<2){std::printf("Pi-AI Lab runtime v0.1\nUsage: %s MODEL.gguf\n",argv[0]);return 0;}
 piai::gguf::Model m;if(!m.open(argv[1])){std::fprintf(stderr,"piai: invalid or unsupported GGUF: %s\n",argv[1]);return 2;}
 std::printf("GGUF v%u, tensors=%zu, data_offset=%llu\n",m.version(),m.tensors().size(),(unsigned long long)m.data_offset());
 for(const auto&t:m.tensors())std::printf("tensor %s rank=%zu type=%u offset=%llu bytes=%llu\n",t.name.c_str(),t.shape.size(),(unsigned)t.type,(unsigned long long)t.offset,(unsigned long long)t.size);
 float x[32],w[32];for(int i=0;i<32;i++){x[i]=1.0f/(i+1);w[i]=(float)((i%7)-3)/4.0f;}float out=0;piai::quant::f32_dot(x,w,32,out);std::printf("kernel:f32_dot=%g\n",out);return 0;
}
