#pragma once
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include "gguf.hpp"
#include "inference.hpp"
namespace piai::api {
struct Response{int status=200;std::string content_type="application/json";std::string body;};
struct Request{std::string method,path,body;std::unordered_map<std::string,std::string> query,headers;};
class Server{
 int fd_=-1; bool running_=false; uint16_t port_=5453;
 std::string storage_="/var/lib/piai/models";
 std::unique_ptr<gguf::Model> model_;
 std::unique_ptr<inference::Engine> engine_;
 std::string loaded_name_;
public:
 ~Server(); bool listen(uint16_t port=5453); void stop(); void serve_once(); void serve_forever(); bool running()const{return running_;} uint16_t port()const{return port_;}
private:
 Response route(const Request&); Response json(int,const std::string&);
 Response load_model(const std::string&); Response unload_model(); Response generate(const Request&,const std::string&); void stream_generate(int,const Request&,const std::string&);
 static std::string url_path(const std::string&); static std::string reason(int); static std::string json_escape(const std::string&);
};
}
