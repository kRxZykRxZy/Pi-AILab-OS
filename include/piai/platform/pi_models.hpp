#pragma once
#include <string>
namespace piai::platform {
enum class PiModel { Unknown, Pi1A, Pi1B, Pi1BPlus, Pi2Bv11, Pi2Bv12, Pi3APlus, Pi3B, Pi3BPlus, Pi4B, Pi400, Pi4CM, Pi5, Pi500, Pi5CM };
enum class CpuIsa { ARMv6, ARMv7, ARMv8, ARMv8A };
struct PiProfile { PiModel model=PiModel::Unknown; CpuIsa isa=CpuIsa::ARMv7; const char* name="Unknown Raspberry Pi"; const char* cpu="Unknown"; unsigned cores=1; unsigned recommended_threads=1; bool neon=false; bool fp16_arithmetic=false; };
PiProfile detect_pi();
const char* model_name(PiModel);
const char* isa_name(CpuIsa);
} // namespace piai::platform
