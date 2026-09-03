#include "piai/platform/pi_models.hpp"
#include <algorithm>
#include <fstream>
#include <string>
#include <thread>
namespace piai::platform {
static std::string cpuinfo(){std::ifstream f("/proc/cpuinfo");return std::string((std::istreambuf_iterator<char>(f)),{});}
static PiProfile profile(PiModel m){
    switch(m){
    case PiModel::Pi1A: case PiModel::Pi1B: return {m,CpuIsa::ARMv6,"Raspberry Pi 1 Model A/B","ARM11",1,1,false,false};
    case PiModel::Pi1BPlus: return {m,CpuIsa::ARMv6,"Raspberry Pi 1 Model B+","ARM11",1,1,false,false};
    case PiModel::Pi2Bv11: return {m,CpuIsa::ARMv7,"Raspberry Pi 2 Model B v1.1","BCM2836 Cortex-A7",4,4,true,false};
    case PiModel::Pi2Bv12: return {m,CpuIsa::ARMv8,"Raspberry Pi 2 Model B v1.2","BCM2837 Cortex-A53",4,4,true,false};
    case PiModel::Pi3APlus: return {m,CpuIsa::ARMv8,"Raspberry Pi 3 Model A+","BCM2837B0 Cortex-A53",4,4,true,false};
    case PiModel::Pi3B: return {m,CpuIsa::ARMv8,"Raspberry Pi 3 Model B","BCM2837 Cortex-A53",4,4,true,false};
    case PiModel::Pi3BPlus: return {m,CpuIsa::ARMv8,"Raspberry Pi 3 Model B+","BCM2837B0 Cortex-A53",4,4,true,false};
    case PiModel::Pi4B: return {m,CpuIsa::ARMv8A,"Raspberry Pi 4 Model B","BCM2711 Cortex-A72",4,4,true,true};
    case PiModel::Pi400: return {m,CpuIsa::ARMv8A,"Raspberry Pi 400","BCM2711 Cortex-A72",4,4,true,true};
    case PiModel::Pi4CM: return {m,CpuIsa::ARMv8A,"Raspberry Pi 4 Compute Module","BCM2711 Cortex-A72",4,4,true,true};
    case PiModel::Pi5: return {m,CpuIsa::ARMv8A,"Raspberry Pi 5","BCM2712 Cortex-A76",4,4,true,true};
    case PiModel::Pi500: return {m,CpuIsa::ARMv8A,"Raspberry Pi 500","BCM2712 Cortex-A76",4,4,true,true};
    case PiModel::Pi5CM: return {m,CpuIsa::ARMv8A,"Raspberry Pi 5 Compute Module","BCM2712 Cortex-A76",4,4,true,true};
    default: return {};
    }
}
PiProfile detect_pi(){
    const std::string s=cpuinfo();
    auto has=[&](const char*x){return s.find(x)!=std::string::npos;};
    PiModel m=PiModel::Unknown;
    if(has("BCM2836"))m=PiModel::Pi2Bv11;
    else if(has("BCM2837B0")){m=has("Model B Plus")?PiModel::Pi3BPlus:PiModel::Pi3APlus;}
    else if(has("BCM2837")){m=has("Model B")?PiModel::Pi3B:PiModel::Pi2Bv12;}
    else if(has("BCM2711")){m=has("Pi 400")?PiModel::Pi400:(has("Compute Module")?PiModel::Pi4CM:PiModel::Pi4B);}
    else if(has("BCM2712")){m=has("Pi 500")?PiModel::Pi500:(has("Compute Module")?PiModel::Pi5CM:PiModel::Pi5);}
    else if(has("BCM2708")||has("BCM2835")){m=has("Model B Plus")?PiModel::Pi1BPlus:PiModel::Pi1B;}
    auto p=profile(m); unsigned n=std::thread::hardware_concurrency(); if(n) p.cores=n; p.recommended_threads=std::max(1u,std::min(p.cores,p.cores)); return p;
}
const char* model_name(PiModel m){return profile(m).name;}
const char* isa_name(CpuIsa i){switch(i){case CpuIsa::ARMv6:return "ARMv6";case CpuIsa::ARMv7:return "ARMv7";case CpuIsa::ARMv8:return "ARMv8";default:return "ARMv8-A";}}
} // namespace piai::platform
