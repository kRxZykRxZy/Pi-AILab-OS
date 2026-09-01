#include "inference.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <string>
namespace piai::inference {
static bool u32(const gguf::Value*v,uint32_t&o){if(!v||v->type!=gguf::Type::UINT32||v->bytes.size()!=4)return false;std::memcpy(&o,v->bytes.data(),4);return true;}
static bool u64(const gguf::Value*v,uint64_t&o){if(!v||v->bytes.size()!=8)return false;std::memcpy(&o,v->bytes.data(),8);return true;}
static bool f32(const gguf::Value*v,float&o){if(!v||v->bytes.size()!=4)return false;std::memcpy(&o,v->bytes.data(),4);return true;}
static bool number(const gguf::Value*v,size_t&o){uint32_t x;if(u32(v,x)){o=x;return true;}uint64_t y;if(u64(v,y)&&y<=SIZE_MAX){o=(size_t)y;return true;}return false;}
static const gguf::Value* meta(const gguf::Model&m,const std::string&base,const std::string&suffix){const auto*v=m.metadata(base+"."+suffix);if(v)return v;return m.metadata("general."+suffix);}
static bool scalar_string(const gguf::Value*v,std::string&out){if(!v||v->type!=gguf::Type::STRING)return false;out=v->string;return true;}
bool detect_architecture(const gguf::Model&m,Architecture&a){
 std::string n;const auto*g=m.metadata("general.architecture");if(!scalar_string(g,n))return false;a=Architecture{};a.name=n;
 std::string p=n+".";
 auto get=[&](const char*s,size_t&dst){const auto*v=m.metadata(p+s);return v&&number(v,dst);};
 get("block_count",a.layers);get("embedding_length",a.hidden);get("attention.head_count",a.heads);get("attention.head_count_kv",a.kv_heads);get("feed_forward_length",a.intermediate);get("context_length",a.context);a.vocab=0;
 if(a.kv_heads==0)a.kv_heads=a.heads;
 const auto*e=m.metadata(p+"attention.layer_norm_rms_epsilon");if(e)f32(e,a.eps);const auto*t=m.metadata(p+"rope.freq_base");if(t)f32(t,a.rope_theta);
 const auto*tv=m.metadata("tokenizer.ggml.tokens");if(tv&&tv->type==gguf::Type::ARRAY)a.vocab=tv->array.size();
 return !a.name.empty()&&a.layers&&a.hidden&&a.heads&&a.vocab;
}
bool Vocabulary::load(const gguf::Model&m){tokens_.clear();const auto*v=m.metadata("tokenizer.ggml.tokens");if(!v||v->type!=gguf::Type::ARRAY)return false;const auto*s=m.metadata("tokenizer.ggml.scores");tokens_.reserve(v->array.size());for(size_t i=0;i<v->array.size();++i){const auto&x=v->array[i];if(x.type!=gguf::Type::STRING)return false;Token t;t.text=x.string;if(s&&s->type==gguf::Type::ARRAY&&i<s->array.size())f32(&s->array[i],t.score);tokens_.push_back(std::move(t));}return !tokens_.empty();}
bool Vocabulary::encode(const std::string&s,std::vector<int32_t>&out)const{out.clear();if(tokens_.empty())return false;size_t p=0;while(p<s.size()){size_t best=0;int32_t id=-1;for(size_t i=0;i<tokens_.size();++i){const std::string&t=tokens_[i].text;if(!t.empty()&&t.size()>best&&t.size()<=s.size()-p&&s.compare(p,t.size(),t)==0){best=t.size();id=(int32_t)i;}}if(id<0){unsigned char c=(unsigned char)s[p++];if(c>=tokens_.size())return false;out.push_back((int32_t)c);}else{out.push_back(id);p+=best;}}return true;}
std::string Vocabulary::decode(int32_t id)const{return id>=0&&static_cast<size_t>(id)<tokens_.size()?tokens_[id].text:std::string{};}
bool KVCache::init(size_t l,size_t h,size_t d,size_t cap){if(!l||!h||!d||!cap||l>SIZE_MAX/(h*d))return false;size_t n=l*h*d*cap;if(n>SIZE_MAX/(sizeof(float)*2)){return false;}k_.assign(n,0);v_.assign(n,0);layers_=l;heads_=h;head_dim_=d;capacity_=cap;used_=0;return true;}
void KVCache::clear(){used_=0;std::fill(k_.begin(),k_.end(),0);std::fill(v_.begin(),v_.end(),0);}
bool Engine::load(const gguf::Model&m){if(!detect_architecture(m,arch_))return false;if(!vocab_.load(m))return false;size_t hd=arch_.heads?arch_.hidden/arch_.heads:0;if(!hd||!arch_.context)return false;return cache_.init(arch_.layers,arch_.kv_heads,hd,arch_.context);}
bool Engine::generate(const std::string&prompt,size_t max_tokens,std::vector<std::string>&out){out.clear();if(prompt.empty()||max_tokens==0)return true;std::vector<int32_t>ids;if(!vocab_.encode(prompt,ids))return false;return false;}
}
