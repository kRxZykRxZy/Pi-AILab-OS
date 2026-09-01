#include "inference.hpp"
namespace piai::inference {
static bool has(const gguf::Model&m,const char*k){return m.metadata(k)!=nullptr;}
bool detect_architecture(const gguf::Model&m,Architecture&a){const auto*v=m.metadata("general.architecture");if(!v||v->type!=gguf::Type::STRING)return false;a.name="gguf";if(has(m,"llama.block_count"))a.name="llama";else if(has(m,"qwen2.block_count"))a.name="qwen2";else if(has(m,"mistral.block_count"))a.name="mistral";else if(has(m,"phi3.block_count"))a.name="phi3";return true;}
}
