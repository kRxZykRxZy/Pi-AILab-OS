#include "gguf.hpp"
#include <cstring>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
namespace piai::gguf {
static bool rd(const uint8_t*&p,const uint8_t*e,void*d,size_t n){if(!p||p>e||n>(size_t)(e-p))return false;if(n&&d)std::memcpy(d,p,n);p+=n;return true;}
template<class T>static bool r(const uint8_t*&p,const uint8_t*e,T&v){return rd(p,e,&v,sizeof(v));}
static bool text(const uint8_t*&p,const uint8_t*e,std::string&s){uint64_t n;if(!r(p,e,n)||n>=(1ull<<26)||n>(uint64_t)(e-p))return false;s.assign((const char*)p,(size_t)n);p+=n;return true;}
static bool skip(const uint8_t*&p,const uint8_t*e,Type t){uint64_t n;switch(t){case Type::UINT8:case Type::INT8:case Type::BOOL:return rd(p,e,nullptr,1);case Type::UINT16:case Type::INT16:return rd(p,e,nullptr,2);case Type::UINT32:case Type::INT32:case Type::FLOAT32:return rd(p,e,nullptr,4);case Type::UINT64:case Type::INT64:case Type::FLOAT64:return rd(p,e,nullptr,8);case Type::STRING:{std::string s;return text(p,e,s);}case Type::ARRAY:{uint32_t at;if(!r(p,e,at)||at>12||!r(p,e,n)||n>(1ull<<24))return false;for(uint64_t i=0;i<n;i++)if(!skip(p,e,(Type)at))return false;return true;}default:return false;}}
Model::~Model(){close();}
void Model::close(){if(map_)munmap(map_,map_size_);map_=nullptr;if(fd_>=0)::close(fd_);fd_=-1;map_size_=data_offset_=version_=0;meta_.clear();tensors_.clear();}
bool Model::open(const std::string&path){close();fd_=::open(path.c_str(),O_RDONLY|O_CLOEXEC);if(fd_<0)return false;struct stat st{};if(fstat(fd_,&st)||st.st_size<24){close();return false;}map_size_=st.st_size;map_=mmap(nullptr,map_size_,PROT_READ,MAP_PRIVATE,fd_,0);if(map_==MAP_FAILED){map_=nullptr;close();return false;}const uint8_t*p=(const uint8_t*)map_,*e=p+map_size_;char magic[4];uint64_t nt=0,nm=0;if(!rd(p,e,magic,4)||memcmp(magic,"GGUF",4)||!r(p,e,version_)||version_<1||version_>3||!r(p,e,nt)||!r(p,e,nm)||nt>1000000||nm>1000000){close();return false;}for(uint64_t i=0;i<nm;i++){std::string k;uint32_t ty;if(!text(p,e,k)||!r(p,e,ty)||ty>12){close();return false;}if(!skip(p,e,(Type)ty)){close();return false;}meta_[std::move(k)]={(Type)ty,{}};}for(uint64_t i=0;i<nt;i++){Tensor t;uint32_t rank=0,ty=0;if(!text(p,e,t.name)||!r(p,e,rank)||rank>64){close();return false;}t.shape.resize(rank);for(auto&d:t.shape)if(!r(p,e,d)){close();return false;}if(!r(p,e,ty)||!r(p,e,t.offset)){close();return false;}t.type=(TensorType)ty;tensors_.push_back(std::move(t));}data_offset_=(uint64_t)(p-(const uint8_t*)map_);data_offset_=(data_offset_+31)&~31ull;return data_offset_<=map_size_;}
const Value*Model::metadata(const std::string&k)const{auto i=meta_.find(k);return i==meta_.end()?nullptr:&i->second;}
const uint8_t*Model::data(const Tensor&t)const{return valid()&&t.offset<map_size_-data_offset_?(const uint8_t*)map_+data_offset_+t.offset:nullptr;}
}
