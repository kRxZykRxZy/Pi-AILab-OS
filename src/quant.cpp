#include "quant.hpp"
#include <cstring>
#include <cmath>
namespace piai::quant {
static float h(uint16_t v){uint32_t s=(v>>15)&1,e=(v>>10)&31,f=v&1023;if(!e)return std::ldexp((float)f,-24);if(e==31)return f?NAN:std::ldexp(1.f,128)*(s?-1.f:1.f);return std::ldexp(1.f+f/1024.f,(int)e-15)*(s?-1.f:1.f);}
void f32_dot(const float*x,const float*w,size_t n,float&o){double a=0;for(size_t i=0;i<n;i++)a+=(double)x[i]*w[i];o=(float)a;}
void q8_0_dot(const float*x,const void*ww,size_t n,float&o){const uint8_t*p=(const uint8_t*)ww;float a=0;for(size_t b=0,i=0;i<n;i+=32,b+=34){uint16_t ds;std::memcpy(&ds,p+b,2);float d=h(ds);for(size_t j=0;j<32&&i+j<n;j++){int8_t q=(int8_t)p[b+2+j];a+=x[i+j]*(d*q);}}o=a;}
void q4_0_dot(const float*x,const void*ww,size_t n,float&o){const uint8_t*p=(const uint8_t*)ww;float a=0;for(size_t b=0,i=0;i<n;i+=32,b+=18){uint16_t ds;std::memcpy(&ds,p+b,2);float d=h(ds);for(size_t j=0;j<32&&i+j<n;j++){uint8_t z=p[b+2+j/2];int q=(j&1)?(z>>4):(z&15);a+=x[i+j]*(d*(q-8));}}o=a;}
void q5_0_dot(const float*x,const void*ww,size_t n,float&o){const uint8_t*p=(const uint8_t*)ww;float a=0;for(size_t b=0,i=0;i<n;i+=32,b+=22){uint16_t ds;std::memcpy(&ds,p+b,2);float d=h(ds);uint32_t qh;std::memcpy(&qh,p+b+2,4);for(size_t j=0;j<32&&i+j<n;j++){uint8_t z=p[b+6+j/2];int q=(j&1)?(z>>4):(z&15);q|=((qh>>j)&1)<<4;a+=x[i+j]*(d*(q-16));}}o=a;}
void q4_1_dot(const float*x,const void*ww,size_t n,float&o){const uint8_t*p=(const uint8_t*)ww;float a=0;for(size_t b=0,i=0;i<n;i+=32,b+=20){uint16_t ds,ms;std::memcpy(&ds,p+b,2);std::memcpy(&ms,p+b+2,2);float d=h(ds),m=h(ms);for(size_t j=0;j<32&&i+j<n;j++){uint8_t z=p[b+4+j/2];int q=(j&1)?z>>4:z&15;a+=x[i+j]*(d*q+m);}}o=a;}
void q5_1_dot(const float*x,const void*ww,size_t n,float&o){const uint8_t*p=(const uint8_t*)ww;float a=0;for(size_t b=0,i=0;i<n;i+=32,b+=24){uint16_t ds,ms;std::memcpy(&ds,p+b,2);std::memcpy(&ms,p+b+2,2);uint32_t qh;std::memcpy(&qh,p+b+4,4);float d=h(ds),m=h(ms);for(size_t j=0;j<32&&i+j<n;j++){uint8_t z=p[b+8+j/2];int q=(j&1)?z>>4:z&15;q|=((qh>>j)&1)<<4;a+=x[i+j]*(d*q+m);}}o=a;}
}
