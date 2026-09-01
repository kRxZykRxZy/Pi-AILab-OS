#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
namespace piai::api {
struct Response { int status=200; std::string content_type="application/json"; std::string body; };
struct Request { std::string method; std::string path; std::string body; std::unordered_map<std::string,std::string> query; };
class Server {
 int fd_=-1; bool running_=false;
 public: ~Server(); bool listen(uint16_t port=8080); void stop(); void serve_once(); bool running() const{return running_;}
 private: Response route(const Request&); Response json(int,const std::string&);
};
}
