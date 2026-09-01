#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
namespace piai::api {
struct Response { int status=200; std::string content_type="application/json"; std::string body; };
struct Request { std::string method; std::string path; std::string body; std::unordered_map<std::string,std::string> query; };
class Server {
 int fd_=-1; bool running_=false; uint16_t port_=5453;
 public: ~Server(); bool listen(uint16_t port=5453); void stop(); void serve_once(); void serve_forever(); bool running() const{return running_;} uint16_t port() const{return port_;}
 private: Response route(const Request&); Response json(int,const std::string&); static std::string url_path(const std::string&); static std::string reason(int);
};
}
