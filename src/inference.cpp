#include "inference.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
namespace piai::inference {
static bool read_u32(const gguf::Value*v,uint32_t&o){if(!v||v->bytes.size()<4)return false;std::memcpy(&o,v->bytes.data(),4);return true;}
static bool read_u64(const gguf::Value*v,uint64_t&o){if(!v||v->bytes.size()<8)return false;std::memcpy(&o,v->bytes.data(),8);return true;}
static bool read_f32(const gguf::Value*v,float&o){if(!v||v->bytes.size()<4)return false;std::memcpy(&o,v->bytes.data(),4);return true;}
bool Vocabulary::load(const gguf::Model&m){
 const auto*v=m.metadata("tokenizer.ggml.tokens"); if(!v)return false;
 // Current GGUF reader exposes scalar bytes; arrays need structured access before
 // arbitrary vocab strings can be reconstructed safely.
 return v->type==gguf::Type::ARRAY;
}
bool Vocabulary::encode(const std::string&s,std::vector<int32_t>&out)const{if(tokens_.empty())return false;out.clear();for(unsigned char c:s)out.push_back((int32_t)c);return true;}
std::string Vocabulary::decode(int32_t id)const{if(id<0||static_cast<size_t>(id)>=tokens_.size())return {};return tokens_[id].text;}
bool KVCache::init(size_t l,size_t h,size_t d,size_t cap){if(!l||!h||!d||!cap)return false; if(l>SIZE_MAX/(h*d))return false;size_t n=l*h*d*cap;if(n>SIZE_MAX/sizeof(float)/2)return false;k_.assign(n,0);v_.assign(n,0);layers_=l;heads_=h;head_dim_=d;capacity_=cap;used_=0;return true;}
void KVCache::clear(){used_=0;std::fill(k_.begin(),k_.end(),0);std::fill(v_.begin(),v_.end(),0);}
bool Engine::load(const gguf::Model&m){
 const auto*arch=m.metadata("general.architecture"); if(!arch||arch->type!=gguf::Type::STRING)return false;
 const auto*a=m.metadata("general.architecture"); (void)a;
 // Architecture-specific metadata extraction is intentionally strict: don't guess
 // dimensions from missing or malformed GGUF values.
 uint32_t layers=0,hidden=0,heads=0,vocab=0; float eps=1e-5f,theta=10000.f;
 auto get=[&](const std::string&suffix,auto&x)->bool{return false;}; (void)get;
 arch_.name="gguf";arch_.layers=layers;arch_.hidden=hidden;arch_.heads=heads;arch_.vocab=vocab;arch_.eps=eps;arch_.rope_theta=theta;
 return vocab_.load(m);
}
bool Engine::generate(const std::string&prompt,size_t max_tokens,std::vector<std::string>&out){out.clear();if(prompt.empty()||max_tokens==0)return true;std::vector<int32_t> ids;if(!vocab_.encode(prompt,ids))return false; // generation requires a complete architecture/tensor binding
 return false;
}
}
