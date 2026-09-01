#include "api.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <cstring>
namespace piai::api {
Server::~Server(){stop();}
Response Server::json(int status,const std::string& body){return {status,"application/json",body};}
bool Server::listen(uint16_t port){if(running_)return true;fd_=::socket(AF_INET,SOCK_STREAM,0);if(fd_<0)return false;int one=1;::setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_LOOPBACK);a.sin_port=htons(port);if(::bind(fd_,(sockaddr*)&a,sizeof(a))<0||::listen(fd_,8)<0){stop();return false;}running_=true;return true;}
void Server::stop(){if(fd_>=0)::close(fd_);fd_=-1;running_=false;}
Response Server::route(const Request&r){
 if(r.method=="GET"&&r.path=="/api/v1/health")return json(200,"{\"status\":\"ok\",\"runtime\":\"piai\"}");
 if(r.method=="GET"&&r.path=="/api/v1/runtime")return json(200,"{\"status\":\"running\"}");
 if(r.method=="GET"&&r.path=="/api/v1/models")return json(200,"{\"models\":[]}");
 if(r.method=="GET"&&r.path=="/api/v1")return json(200,"{\"name\":\"Pi-AI Lab API\",\"version\":\"v1\"}");
 return json(404,"{\"error\":\"not_found\"}");
}
void Server::serve_once(){if(!running_)return;int c=::accept(fd_,nullptr,nullptr);if(c<0)return;char buf[8192];ssize_t n=::recv(c,buf,sizeof(buf)-1,0);if(n<=0){::close(c);return;}buf[n]=0;std::istringstream in(std::string(buf,n));Request r;in>>r.method>>r.path;Response out=route(r);std::string h="HTTP/1.1 "+std::to_string(out.status)+(out.status==200?" OK":" Not Found")+"\r\nContent-Type: "+out.content_type+"\r\nContent-Length: "+std::to_string(out.body.size())+"\r\nConnection: close\r\n\r\n"+out.body;::send(c,h.data(),h.size(),0);::close(c);}
}
