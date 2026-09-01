#include "piai/http.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <string>
namespace piai { static void client(int fd,Runtime& r){(void)r;char b[1024];ssize_t n=read(fd,b,sizeof(b)-1);if(n<0)n=0;b[n]=0;const char* body="{\"status\":\"ok\",\"service\":\"piai\"}\n";if(std::strncmp(b,"GET /api/v1/health",19)==0){std::string h="HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "+std::to_string(std::strlen(body))+"\r\nConnection: close\r\n\r\n"+body;write(fd,h.data(),h.size());}else{const char* x="HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";write(fd,x,std::strlen(x));}close(fd);} bool http_run(Runtime& runtime,const char* address,unsigned port){int s=socket(AF_INET,SOCK_STREAM,0);if(s<0)return false;int one=1;setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));sockaddr_in a{};a.sin_family=AF_INET;a.sin_port=htons((uint16_t)port);if(inet_pton(AF_INET,address,&a.sin_addr)!=1){close(s);return false;}if(::bind(s,(sockaddr*)&a,sizeof(a))<0||listen(s,16)<0){close(s);return false;}while(runtime.running()){int c=accept(s,nullptr,nullptr);if(c>=0)std::thread(client,c,std::ref(runtime)).detach();}close(s);return true;} }
