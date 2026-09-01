#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace piai::gguf {
enum class Type:uint32_t{UINT8=0,INT8=1,UINT16=2,INT16=3,UINT32=4,INT32=5,FLOAT32=6,BOOL=7,STRING=8,ARRAY=9,UINT64=10,INT64=11,FLOAT64=12};
enum class TensorType:uint32_t{F32=0,F16=1,Q4_0=2,Q4_1=3,Q5_0=6,Q5_1=7,Q8_0=8,Q8_1=9};
struct Value{Type type{};std::vector<uint8_t> bytes;std::vector<Value> array;std::string string;bool is_array()const{return type==Type::ARRAY;}bool is_string()const{return type==Type::STRING;}};
struct Tensor{std::string name;std::vector<uint64_t> shape;TensorType type{};uint64_t offset=0,size=0;};
class Model{int fd_=-1;void*map_=nullptr;size_t map_size_=0;uint64_t data_offset_=0;uint32_t version_=0;std::unordered_map<std::string,Value>meta_;std::vector<Tensor>tensors_;public:Model()=default;~Model();Model(const Model&)=delete;Model&operator=(const Model&)=delete;bool open(const std::string&);void close();bool valid()const{return map_!=nullptr;}uint32_t version()const{return version_;}uint64_t data_offset()const{return data_offset_;}const std::vector<Tensor>&tensors()const{return tensors_;}const Value*metadata(const std::string&)const;const uint8_t*data(const Tensor&)const;size_t size()const{return map_size_;}};
}
