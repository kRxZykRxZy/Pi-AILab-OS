#include "tensor.h"
namespace pilab::tensor {
void matmul(const View&a,const View&b,View&o){for(uint32_t i=0;i<a.rows;i++)for(uint32_t j=0;j<b.cols;j++){float s=0;for(uint32_t k=0;k<a.cols;k++)s+=a.data[i*a.cols+k]*b.data[k*b.cols+j];o.data[i*o.cols+j]=s;}}
void add(const View&a,const View&b,View&o){uint32_t n=a.rows*a.cols;for(uint32_t i=0;i<n;i++)o.data[i]=a.data[i]+b.data[i];}
void relu(View&x){uint32_t n=x.rows*x.cols;for(uint32_t i=0;i<n;i++)if(x.data[i]<0)x.data[i]=0;}
}
