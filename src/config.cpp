#include "piai/config.h"
#include <fstream>
namespace piai { Config load_config(const char* path){ Config c; if(!path) return c; std::ifstream f(path); std::string k,v; while(f>>k>>v){ if(k=="bind") c.bind=v; else if(k=="port") c.port=(unsigned)std::stoul(v); else if(k=="model_dir") c.model_dir=v; } return c; } }
