#include <stdint.h>
namespace pilab::uart { void putc(char); char getc(); void write(const char*); }
namespace pilab::shell {
static bool eq(const char*a,const char*b){while(*a&&*b&&*a==*b){++a;++b;}return *a==0&&*b==0;}
static void line(char* b,uint32_t n){uint32_t p=0;for(;;){char c=pilab::uart::getc();if(c=='\r'||c=='\n'){pilab::uart::write("\r\n");b[p]=0;return;}if((c=='\b'||c==127)&&p){--p;pilab::uart::write("\b \b");continue;}if(c>=32&&c<127&&p+1<n){b[p++]=c;pilab::uart::putc(c);}}}
void run(){char b[96];pilab::uart::write("\r\nPiLab OS v0.1\r\nType 'help'.\r\n\r\nPiLab> ");for(;;){line(b,sizeof(b));if(eq(b,"help")){pilab::uart::write("help  info  ai  clear  reboot\r\n");}else if(eq(b,"info")){pilab::uart::write("Pi AI Lab native ARM kernel\r\n");}else if(eq(b,"ai")){pilab::uart::write("AI engine: online\r\n");}else if(eq(b,"clear")){for(int i=0;i<40;i++)pilab::uart::write("\r\n");}else if(eq(b,"reboot")){pilab::uart::write("reboot unavailable in v0.1\r\n");}else if(b[0]){pilab::uart::write("unknown command\r\n");}pilab::uart::write("PiLab> ");}}
}
