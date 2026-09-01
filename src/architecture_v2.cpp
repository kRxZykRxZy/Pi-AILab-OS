#include "inference.hpp"
#include <cstring>
namespace piai::inference {
static bool u32(const gguf::Value*v,uint32_t&o){if(!v||v->bytes.size()<4)return false;std::memcpy(&o,v->bytes.data(),4);return true;}
static bool f32(const gguf::Value*v,float&o){if(!v||v->bytes.size()<4)return false;std::memcpy(&o,v->bytes.data(),4);return true;}
static bool g32(const gguf::Model&m,const std::string&k,size_t&o){uint32_t x;if(!u32(m.metadata(k),x))return false;o=x;return true;}
bool detect_architecture(const gguf::Model&m,Architecture&a){const auto*g=m.metadata("general.architecture");if(!g||!g->is_string())return false;a=Architecture{};a.name=g->string;const std::string p=a.name+".";if(!g32(m,p+"block_count",a.layers)||!g32(m,p+"embedding_length",a.hidden)||!g32(m,p+"attention.head_count",a.heads)||!g32(m,p+"feed_forward_length",a.intermediate))return false;if(!g32(m,p+"attention.head_count_kv",a.kv_heads))a.kv_heads=a.heads;if(!g32(m,p+"context_length",a.context))a.context=2048;const auto*t=m.metadata("tokenizer.ggml.tokens");if(!t||!t->is_array())return false;a.vocab=t->array.size();float x;if(f32(m.metadata(p+"attention.layer_norm_rms_epsilon"),x))a.eps=x;if(f32(m.metadata(p+"rope.freq_base"),x))a.rope_theta=x;return a.layers&&a.hidden&&a.heads&&a.kv_heads&&a.vocab&&a.intermediate&&a.hidden%a.heads==0&&a.heads%a.kv_heads==0;}
}
