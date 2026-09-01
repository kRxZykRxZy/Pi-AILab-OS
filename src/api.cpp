#include "api.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cerrno>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <regex>
namespace fs=std::filesystem;
namespace piai::api {
Server::~Server(){stop();}
Response Server::json(int s,const std::string&b){return{s,"application/json",b};}
std::string Server::reason(int s){switch(s){case 200:return"OK";case 201:return"Created";case 400:return"Bad Request";case 404:return"Not Found";case 405:return"Method Not Allowed";case 409:return"Conflict";case 500:return"Internal Server Error";default:return"Error";}}
std::string Server::url_path(const std::string&p){auto q=p.find('?');return q==std::string::npos?p:p.substr(0,q);}
bool Server::listen(uint16_t port){if(running_)return true;port_=port;fd_=socket(AF_INET,SOCK_STREAM,0);if(fd_<0)return false;int one=1;setsockopt(fd_,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));sockaddr_in a{};a.sin_family=AF_INET;a.sin_addr.s_addr=htonl(INADDR_ANY);a.sin_port=htons(port_);if(bind(fd_,(sockaddr*)&a,sizeof(a))<0||::listen(fd_,16)<0){stop();return false;}try{fs::create_directories(storage_);}catch(...){stop();return false;}running_=true;return true;}
void Server::stop(){if(fd_>=0)::close(fd_);fd_=-1;running_=false;}
Response Server::route(const Request&r){std::string p=url_path(r.path);
 if(r.method=="GET"&&p=="/api/v1/health")return json(200,"{\"status\":\"ok\",\"runtime\":\"piai\"}");
 if(r.method=="GET"&&p=="/api/v1/models"){std::string b="{\"models\":[";bool first=true;try{for(auto&e:fs::directory_iterator(storage_)){if(!e.is_regular_file())continue;if(!first)b+=',';first=false;b+="{\"name\":\""+e.path().filename().string()+"\",\"loaded\":false}";}}catch(...){}b+="]}";return json(200,b);}
 if(r.method=="POST"&&p=="/api/v1/models/upload"){
   auto ct=r.headers.find("content-type");if(ct==r.headers.end())return json(400,"{\"error\":\"missing_content_type\"}");
   auto bp=ct->second.find("boundary=");if(bp==std::string::npos)return json(400,"{\"error\":\"multipart_required\"}");std::string boundary="--"+ct->second.substr(bp+9);auto pos=r.body.find(boundary);if(pos==std::string::npos)return json(400,"{\"error\":\"invalid_multipart\"}");auto hs=r.body.find("\r\n\r\n",pos);if(hs==std::string::npos)return json(400,"{\"error\":\"invalid_multipart\"}");hs+=4;auto end=r.body.find("\r\n"+boundary,hs);if(end==std::string::npos)return json(400,"{\"error\":\"invalid_multipart\"}");auto h=r.body.substr(pos,hs-pos);std::smatch m;if(!std::regex_search(h,m,std::regex("filename=\\\"([^\\\"]+)\\\"")))return json(400,"{\"error\":\"missing_filename\"}");std::string name=m[1].str();for(char&c:name)if(!(std::isalnum((unsigned char)c)||c=='.'||c=='-'||c=='_'))c='_';if(name.size()<5||name.substr(name.size()-5)!=".gguf")return json(400,"{\"error\":\"gguf_required\"}");fs::path out=fs::path(storage_)/name;std::ofstream f(out,std::ios::binary|std::ios::trunc);if(!f)return json(500,"{\"error\":\"storage_error\"}");f.write(r.body.data()+hs,(std::streamsize)(end-hs));f.close();return json(201,"{\"name\":\""+name+"\",\"stored\":true,\"loaded\":false}");}
 if(r.method=="POST"&&p.rfind("/api/v1/models/",0)==0&&p.size()>17&&p.substr(p.size()-5)=="/load"){return json(501,"{\"error\":\"model_load_not_wired\"}");}
 return json(404,"{\"error\":\"not_found\"}");}
void Server::serve_once(){if(!running_)return;int c=accept(fd_,nullptr,nullptr);if(c<0)return;std::string raw;char buf[65536];for(;;){ssize_t n=recv(c,buf,sizeof(buf),0);if(n<=0)break;raw.append(buf,n);auto he=raw.find("\r\n\r\n");if(he!=std::string::npos){auto cl=raw.find("Content-Length:");size_t need=0;if(cl!=std::string::npos){auto e=raw.find("\r\n",cl);need=std::stoull(raw.substr(cl+15,e-cl-15));}if(raw.size()>=(he+4+need))break;}if(raw.size()>128ull*1024*1024){::close(c);return;}}
auto he=raw.find("\r\n\r\n");if(he==std::string::npos){::close(c);return;}std::istringstream in(raw.substr(0,he));Request r;in>>r.method>>r.path;std::string line;std::getline(in,line);while(std::getline(in,line)&&line!="\r"){if(!line.empty()&&line.back()=='\r')line.pop_back();auto x=line.find(':');if(x!=std::string::npos){std::string k=line.substr(0,x),v=line.substr(x+1);while(!v.empty()&&v[0]==' ')v.erase(0,1);std::transform(k.begin(),k.end(),k.begin(),[](unsigned char x){return std::tolower(x);});r.headers[k]=v;}}r.body=raw.substr(he+4);Response out=route(r);std::string h="HTTP/1.1 "+std::to_string(out.status)+" "+reason(out.status)+"\r\nContent-Type: "+out.content_type+"\r\nContent-Length: "+std::to_string(out.body.size())+"\r\nConnection: close\r\n\r\n"+out.body;send(c,h.data(),h.size(),MSG_NOSIGNAL);::close(c);}
void Server::serve_forever(){while(running_)serve_once();}
}
