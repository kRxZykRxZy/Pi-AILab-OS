#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
namespace piai::transformer {
struct Config { size_t layers=0, hidden=0, heads=0, kv_heads=0, intermediate=0, vocab=0; float rms_eps=1e-5f, rope_theta=10000.f; };
struct Layer { std::vector<float> wq,wk,wv,wo,w1,w2,w3,norm1,norm2; };
class Model {
 Config cfg_{}; std::vector<Layer> layers_;
 public: bool init(const Config& c); bool forward(const std::vector<float>& input,std::vector<float>& output) const; const Config& config() const{return cfg_;}
};
void rmsnorm(const float*,float*,size_t,const float*,float);
void rope(float*,size_t,size_t,float);
void softmax(float*,size_t);
}
