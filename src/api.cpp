#include "api.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <cstring>
#include <algorithm>
namespace piai::api {
Server::~Server(){stop();}
Response Server::json(int status,const std::string& body){return {status,"application/json",body};}
std::string Server::reason(int s){switch(s){case 200:return "OK";case 201:return "Created";case 204:return "No Content";case 400:return "Bad Request";case 404:return "Not Found";case 405:return "Method Not Allowed";case 500:return "Internal Server Error";default:return "Error";}}
std::string Server::url_path(const std::string& p){auto q=p.find('?');return q==std::string::npos?p:p.substr(0,q);}
bool Server::listen(uint16_t port){if(running_)return true;port_=port;fd_=::socket(AF_INET,SOCK_STREAM,0);if(fd_<0)return false;int one=1;::setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_ANY);a.sin_port=htons(port_);if(::bind(fd_,(sockaddr*)&a,sizeof(a))<0||::listen(fd_,16)<0){stop();return false;}running_=true;return true;}
void Server::stop(){if(fd_>=0)::close(fd_);fd_=-1;running_=false;}
Response Server::route(const Request&r){const auto p=url_path(r.path);
 if(r.method=="GET"&&p=="/api")return json(200,"{\"name\":\"Pi-AI Lab API\",\"version\":\"v1\"}");
 if(r.method=="GET"&&p=="/api/v1")return json(200,"{\"name\":\"Pi-AI Lab API\",\"version\":\"v1\"}");
 if(r.method=="GET"&&p=="/api/v1/health")return json(200,"{\"status\":\"ok\",\"runtime\":\"piai\"}");
 if(r.method=="GET"&&p=="/api/v1/ready")return json(200,"{\"ready\":true}");
 if(r.method=="GET"&&p=="/api/v1/runtime")return json(200,"{\"status\":\"running\"}");
 if(r.method=="GET"&&p=="/api/v1/runtime/status")return json(200,"{\"status\":\"running\"}");
 if(r.method=="GET"&&p=="/api/v1/models")return json(200,"{\"models\":[]}");
 if(r.method=="GET"&&p=="/api/v1/system")return json(200,"{\"runtime\":\"piai\",\"api_port\":5453}");
 if(r.method=="GET"&&p=="/api/v1/config")return json(200,"{\"port\":5453}");
 if(r.method=="GET"&&p=="/api/v1/metrics")return json(200,"{\"requests_total\":0}");
 return json(404,"{\"error\":\"not_found\"}");}
void Server::serve_once(){if(!running_)return;int c=::accept(fd_,nullptr,nullptr);if(c<0)return;char buf[8192];ssize_t n=::recv(c,buf,sizeof(buf)-1,0);if(n<=0){::close(c);return;}buf[n]=0;std::istringstream in(std::string(buf,n));Request r;in>>r.method>>r.path;Response out=route(r);std::string h="HTTP/1.1 "+std::to_string(out.status)+" "+reason(out.status)+"\r\nContent-Type: "+out.content_type+"\r\nContent-Length: "+std::to_string(out.body.size())+"\r\nConnection: close\r\n\r\n"+out.body;::send(c,h.data(),h.size(),MSG_NOSIGNAL);::close(c);}
void Server::serve_forever(){while(running_)serve_once();}
}
