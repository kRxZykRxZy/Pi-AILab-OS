#include "transformer.hpp"
#include <algorithm>
#include <cmath>
#include <cstring>
namespace piai::transformer {
void rmsnorm(const float* x,float* y,size_t n,const float* weight,float eps){ double s=0; for(size_t i=0;i<n;i++) s+=(double)x[i]*x[i]; float inv=1.f/std::sqrt((float)(s/n)+eps); for(size_t i=0;i<n;i++) y[i]=x[i]*inv*(weight?weight[i]:1.f); }
void rope(float* x,size_t n,size_t pos,float theta){ if(!x||n<2)return; for(size_t i=0;i+1<n;i+=2){float inv=std::pow(theta,-2.f*(float)(i/2)/(float)n),a=(float)pos*inv,c=std::cos(a),s=std::sin(a),u=x[i],v=x[i+1];x[i]=u*c-v*s;x[i+1]=u*s+v*c;} }
void softmax(float* x,size_t n){ if(!x||!n)return; float m=*std::max_element(x,x+n),sum=0;for(size_t i=0;i<n;i++){x[i]=std::exp(x[i]-m);sum+=x[i];}if(sum)for(size_t i=0;i<n;i++)x[i]/=sum; }
static void matvec(const std::vector<float>& w,const float* x,float* y,size_t rows,size_t cols){for(size_t r=0;r<rows;r++){double s=0;const float*p=w.data()+r*cols;for(size_t c=0;c<cols;c++)s+=(double)p[c]*x[c];y[r]=(float)s;}}
bool Model::init(const Config& c){if(!c.hidden||!c.layers||!c.heads||!c.vocab||c.hidden%c.heads)return false;cfg_=c;layers_.resize(c.layers);return true;}
bool Model::forward(const std::vector<float>& input,std::vector<float>& output) const {if(!cfg_.hidden||input.size()!=cfg_.hidden)return false;std::vector<float>x=input,tmp(cfg_.hidden),attn(cfg_.hidden);for(const auto&l:layers_){if(l.norm1.size()==cfg_.hidden)rmsnorm(x.data(),tmp.data(),cfg_.hidden,l.norm1.data(),cfg_.rms_eps);else tmp=x;if(l.wq.size()==cfg_.hidden*cfg_.hidden){matvec(l.wq,tmp.data(),attn.data(),cfg_.hidden,cfg_.hidden);rope(attn.data(),cfg_.hidden,0,cfg_.rope_theta);}else attn=tmp;if(l.wo.size()==cfg_.hidden*cfg_.hidden){matvec(l.wo,attn.data(),tmp.data(),cfg_.hidden,cfg_.hidden);for(size_t i=0;i<cfg_.hidden;i++)x[i]+=tmp[i];}else x=attn;if(l.norm2.size()==cfg_.hidden)rmsnorm(x.data(),tmp.data(),cfg_.hidden,l.norm2.data(),cfg_.rms_eps);else tmp=x;if(l.w1.size()==cfg_.intermediate*cfg_.hidden && l.w2.size()==cfg_.hidden*cfg_.intermediate){std::vector<float>h(cfg_.intermediate);matvec(l.w1,tmp.data(),h.data(),cfg_.intermediate,cfg_.hidden);for(float&v:h)v=v/(1.f+std::exp(-v));matvec(l.w2,h.data(),tmp.data(),cfg_.hidden,cfg_.intermediate);for(size_t i=0;i<cfg_.hidden;i++)x[i]+=tmp[i];}}output=x;return true;}
}
