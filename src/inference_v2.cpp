#include "inference.hpp"
#include <cstring>
namespace piai::inference {
static bool f32(const gguf::Value*v,float&o){if(!v||v->bytes.size()<4)return false;std::memcpy(&o,v->bytes.data(),4);return true;}
bool Vocabulary::load(const gguf::Model&m){tokens_.clear();const auto*v=m.metadata("tokenizer.ggml.tokens");if(!v||!v->is_array())return false;const auto*s=m.metadata("tokenizer.ggml.scores");for(size_t i=0;i<v->array.size();++i){if(!v->array[i].is_string())return false;Token t;t.text=v->array[i].string;if(s&&s->is_array()&&i<s->array.size())f32(&s->array[i],t.score);tokens_.push_back(std::move(t));}return !tokens_.empty();}
bool Vocabulary::encode(const std::string&s,std::vector<int32_t>&out)const{out.clear();if(tokens_.empty())return false;for(size_t p=0;p<s.size();){int best=-1;size_t bl=0;for(size_t i=0;i<tokens_.size();++i){const auto&t=tokens_[i].text;if(t.size()<=bl||t.size()>s.size()-p)continue;if(std::memcmp(s.data()+p,t.data(),t.size())==0){best=(int)i;bl=t.size();}}if(best<0)return false;p+=bl;out.push_back(best);}return true;}
std::string Vocabulary::decode(int32_t id)const{return id>=0&&static_cast<size_t>(id)<tokens_.size()?tokens_[id].text:std::string{};}
bool KVCache::init(size_t l,size_t h,size_t d,size_t cap){if(!l||!h||!d||!cap)return false;if(l>SIZE_MAX/(h*d)||l*h*d>SIZE_MAX/cap)return false;size_t n=l*h*d*cap;if(n>SIZE_MAX/(2*sizeof(float)))return false;k_.assign(n,0);v_.assign(n,0);layers_=l;heads_=h;head_dim_=d;capacity_=cap;used_=0;return true;}
void KVCache::clear(){used_=0;}
bool Engine::load(const gguf::Model&m){if(!detect_architecture(m,arch_)||!vocab_.load(m))return false;return cache_.init(arch_.layers,arch_.kv_heads,arch_.hidden/arch_.heads,arch_.context);}
bool Engine::generate(const std::string&,size_t,std::vector<std::string>&out){out.clear();return false;}
}
