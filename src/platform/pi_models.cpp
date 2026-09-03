#include "piai/platform/pi_models.hpp"
#include <algorithm>
#include <fstream>
#include <string>
#include <thread>
namespace piai::platform {
static std::string read_text(const char*path){std::ifstream f(path,std::ios::binary);return std::string((std::istreambuf_iterator<char>(f)),{});}
static PiProfile profile(PiModel m){
    switch(m){
    case PiModel::Pi1A:return {m,CpuIsa::ARMv6,"Raspberry Pi 1 Model A","ARM11",1,1,false,false};
    case PiModel::Pi1B:return {m,CpuIsa::ARMv6,"Raspberry Pi 1 Model B","ARM11",1,1,false,false};
    case PiModel::Pi1BPlus:return {m,CpuIsa::ARMv6,"Raspberry Pi 1 Model B+","ARM11",1,1,false,false};
    case PiModel::Pi2Bv11:return {m,CpuIsa::ARMv7,"Raspberry Pi 2 Model B v1.1","BCM2836 Cortex-A7",4,4,true,false};
    case PiModel::Pi2Bv12:return {m,CpuIsa::ARMv8,"Raspberry Pi 2 Model B v1.2","BCM2837 Cortex-A53",4,4,true,false};
    case PiModel::Pi3APlus:return {m,CpuIsa::ARMv8,"Raspberry Pi 3 Model A+","BCM2837B0 Cortex-A53",4,4,true,false};
    case PiModel::Pi3B:return {m,CpuIsa::ARMv8,"Raspberry Pi 3 Model B","BCM2837 Cortex-A53",4,4,true,false};
    case PiModel::Pi3BPlus:return {m,CpuIsa::ARMv8,"Raspberry Pi 3 Model B+","BCM2837B0 Cortex-A53",4,4,true,false};
    case PiModel::Pi4B:return {m,CpuIsa::ARMv8A,"Raspberry Pi 4 Model B","BCM2711 Cortex-A72",4,4,true,true};
    case PiModel::Pi400:return {m,CpuIsa::ARMv8A,"Raspberry Pi 400","BCM2711 Cortex-A72",4,4,true,true};
    case PiModel::Pi4CM:return {m,CpuIsa::ARMv8A,"Raspberry Pi 4 Compute Module","BCM2711 Cortex-A72",4,4,true,true};
    case PiModel::Pi5:return {m,CpuIsa::ARMv8A,"Raspberry Pi 5","BCM2712 Cortex-A76",4,4,true,true};
    case PiModel::Pi500:return {m,CpuIsa::ARMv8A,"Raspberry Pi 500","BCM2712 Cortex-A76",4,4,true,true};
    case PiModel::Pi5CM:return {m,CpuIsa::ARMv8A,"Raspberry Pi 5 Compute Module","BCM2712 Cortex-A76",4,4,true,true};
    default:return {};
    }
}
PiProfile detect_pi(){
    const std::string model=read_text("/proc/device-tree/model"),info=read_text("/proc/cpuinfo");
    auto has=[](const std::string&s,const char*x){return s.find(x)!=std::string::npos;}; PiModel m=PiModel::Unknown;
    if(has(model,"Pi 1 Model A"))m=PiModel::Pi1A; else if(has(model,"Pi 1 Model B Plus")||has(model,"Pi 1 Model B+"))m=PiModel::Pi1BPlus; else if(has(model,"Pi 1 Model B"))m=PiModel::Pi1B;
    else if(has(model,"Pi 2 Model B"))m=has(model,"v1.2")?PiModel::Pi2Bv12:PiModel::Pi2Bv11;
    else if(has(model,"Pi 3 Model A Plus")||has(model,"Pi 3 Model A+"))m=PiModel::Pi3APlus; else if(has(model,"Pi 3 Model B Plus")||has(model,"Pi 3 Model B+"))m=PiModel::Pi3BPlus; else if(has(model,"Pi 3 Model B"))m=PiModel::Pi3B;
    else if(has(model,"Pi 4 Model B"))m=PiModel::Pi4B; else if(has(model,"Pi 400"))m=PiModel::Pi400; else if(has(model,"Pi 4 Compute Module"))m=PiModel::Pi4CM;
    else if(has(model,"Pi 5 Compute Module"))m=PiModel::Pi5CM; else if(has(model,"Pi 500"))m=PiModel::Pi500; else if(has(model,"Pi 5"))m=PiModel::Pi5;
    else if(has(info,"BCM2836"))m=PiModel::Pi2Bv11; else if(has(info,"BCM2837B0"))m=PiModel::Pi3BPlus; else if(has(info,"BCM2837"))m=PiModel::Pi3B; else if(has(info,"BCM2711"))m=PiModel::Pi4B; else if(has(info,"BCM2712"))m=PiModel::Pi5; else if(has(info,"BCM2708")||has(info,"BCM2835"))m=PiModel::Pi1B;
    auto p=profile(m);unsigned n=std::thread::hardware_concurrency();if(n)p.cores=n;p.recommended_threads=std::max(1u,p.cores);return p;
}
const char*model_name(PiModel m){return profile(m).name;}
const char*isa_name(CpuIsa i){switch(i){case CpuIsa::ARMv6:return "ARMv6";case CpuIsa::ARMv7:return "ARMv7";case CpuIsa::ARMv8:return "ARMv8";default:return "ARMv8-A";}}
} // namespace piai::platform
