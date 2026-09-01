#pragma once
#include "gguf.hpp"
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <vector>
namespace piai::inference {
struct Architecture { std::string name; size_t layers=0,hidden=0,heads=0,kv_heads=0,intermediate=0,vocab=0,context=0; float eps=1e-5f,rope_theta=10000.f; };
bool detect_architecture(const gguf::Model&,Architecture&);
struct Token {std::string text;float score=0;};
class Vocabulary {std::vector<Token>tokens_;public:bool load(const gguf::Model&);bool encode(const std::string&,std::vector<int32_t>&)const;std::string decode(int32_t)const;size_t size()const{return tokens_.size();}};
struct Sampling {float temperature=.8f;int top_k=40;float top_p=.95f;float repeat_penalty=1.1f;uint64_t seed=0;};
class KVCache {size_t layers_=0,heads_=0,head_dim_=0,capacity_=0,used_=0;std::vector<float>k_,v_;public:bool init(size_t,size_t,size_t,size_t);void clear();size_t used()const{return used_;}size_t capacity()const{return capacity_;}float* key(size_t,size_t,size_t);float* value(size_t,size_t,size_t);};
struct TensorBinding {const gguf::Tensor*tensor=nullptr;const uint8_t*data=nullptr;};
class Engine {Architecture arch_;Vocabulary vocab_;KVCache cache_;Sampling sampling_;std::mt19937_64 rng_;const gguf::Model*model_=nullptr;TensorBinding embedding_{},output_{},norm_final_{};std::vector<TensorBinding>q_,k_,v_,o_,norm1_,norm2_,ffn1_,ffn2_,ffn3_;public:bool load(const gguf::Model&);bool generate(const std::string&,size_t,std::vector<std::string>&);const Architecture&architecture()const{return arch_;}const TensorBinding&embedding()const{return embedding_;}};
}
