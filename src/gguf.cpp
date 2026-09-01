#include "gguf.hpp"
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
namespace piai::gguf {
static bool rd(const uint8_t*&p,const uint8_t*e,void*d,size_t n){if(n>(size_t)(e-p))return false;std::memcpy(d,p,n);p+=n;return true;}
template<class T> static bool scalar(const uint8_t*&p,const uint8_t*e,T&v){return rd(p,e,&v,sizeof v);}
static bool str(const uint8_t*&p,const uint8_t*e,std::string&s){uint64_t n;if(!scalar(p,e,n)||n>(uint64_t)(e-p)||n>(1ull<<26))return false;s.assign((const char*)p,(size_t)n);p+=n;return true;}
static bool skip_value(const uint8_t*&p,const uint8_t*e,Type t){uint64_t n;switch(t){case Type::UINT8:case Type::INT8:case Type::BOOL:return rd(p,e,nullptr,1);case Type::UINT16:case Type::INT16:return rd(p,e,nullptr,2);case Type::UINT32:case Type::INT32:case Type::FLOAT32:return rd(p,e,nullptr,4);case Type::UINT64:case Type::INT64:case Type::FLOAT64:return rd(p,e,nullptr,8);case Type::STRING:return str(p,e,*new std::string);case Type::ARRAY:{uint32_t at;if(!scalar(p,e,at)||!scalar(p,e,n)||n>(1ull<<24))return false;for(uint64_t i=0;i<n;i++)if(!skip_value(p,e,(Type)at))return false;return true;}default:return false;}}
Model::~Model(){close();}
void Model::close(){if(map_){munmap(map_,map_size_);map_=nullptr;}if(fd_>=0){::close(fd_);fd_=-1;}map_size_=0;tensors_.clear();meta_.clear();}
bool Model::open(const std::string& path){close();fd_=::open(path.c_str(),O_RDONLY|O_CLOEXEC);if(fd_<0)return false;struct stat st{};if(fstat(fd_,&st)||st.st_size<24){close();return false;}map_size_=(size_t)st.st_size;map_=mmap(nullptr,map_size_,PROT_READ,MAP_PRIVATE,fd_,0);if(map_==MAP_FAILED){map_=nullptr;close();return false;}madvise(map_,map_size_,MADV_SEQUENTIAL|MADV_WILLNEED);const uint8_t*p=(const uint8_t*)map_,*e=p+map_size_;char magic[4];uint64_t nmeta,ntensors; if(!rd(p,e,magic,4)||std::memcmp(magic,"GGUF",4)||!scalar(p,e,version_)||version_<1||version_>3||!scalar(p,e,ntensors)||!scalar(p,e,nmeta)||ntensors>(1ull<<20)||nmeta>(1ull<<20)){close();return false;}
for(uint64_t i=0;i<nmeta;i++){std::string k;uint32_t ty;if(!str(p,e,k)||!scalar(p,e,ty)){close();return false;}size_t begin=(size_t)(p-(const uint8_t*)map_);if(!skip_value(p,e,(Type)ty)){close();return false;}size_t end=(size_t)(p-(const uint8_t*)map_);Value v;v.type=(Type)ty;v.bytes.assign((const uint8_t*)map_+begin,(const uint8_t*)map_+end);meta_.emplace(std::move(k),std::move(v));}
for(uint64_t i=0;i<ntensors;i++){Tensor t;uint32_t rank,tt;if(!str(p,e,t.name)||!scalar(p,e,rank)||rank>64){close();return false;}t.shape.resize(rank);for(auto&d:t.shape)if(!scalar(p,e,d)){close();return false;}if(!scalar(p,e,tt)||tt>255||!scalar(p,e,t.offset)){close();return false;}t.type=(TensorType)tt;tensors_.push_back(std::move(t));}
uint64_t align=32;auto it=meta_.find("general.alignment");if(it!=meta_.end()&&it->second.bytes.size()==4)std::memcpy(&align,it->second.bytes.data()+4,4);if(!align||align>4096){close();return false;}data_offset_=(uint64_t)((((uintptr_t)p-(uintptr_t)map_)+align-1)/align*align);if(data_offset_>map_size_){close();return false;}
for(auto&t:tensors_){uint64_t elems=1;for(auto d:t.shape){if(d&&elems>UINT64_MAX/d){close();return false;}elems*=d;}uint64_t bits=0;switch(t.type){case TensorType::F32:bits=32;break;case TensorType::F16:bits=16;break;case TensorType::Q4_0:case TensorType::Q4_1:bits=(elems+1)/2*8*8/(elems?elems:1);break;default:break;}if(t.type==TensorType::Q4_0||t.type==TensorType::Q4_1){if(elems%32){close();return false;}t.size=(elems/32)*18;}else if(t.type==TensorType::Q5_0||t.type==TensorType::Q5_1)t.size=(elems/32)*22;else if(t.type==TensorType::Q8_0)t.size=(elems/32)*34;else {if(!bits||elems>UINT64_MAX/(bits/8)){close();return false;}t.size=elems*(bits/8);}if(t.offset>map_size_-data_offset_||t.size>map_size_-data_offset_-t.offset){close();return false;}}
return true;}
const Value* Model::metadata(const std::string&k)const{auto i=meta_.find(k);return i==meta_.end()?nullptr:&i->second;}
const uint8_t* Model::data(const Tensor&t)const{return valid()&&t.offset<=map_size_-data_offset_?((const uint8_t*)map_+data_offset_+t.offset):nullptr;}
}
