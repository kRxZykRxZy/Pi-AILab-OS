cmake_minimum_required(VERSION 3.16)
project(piai LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
option(PIAI_NATIVE "Tune for the CPU performing the build" ON)
option(PIAI_VC4_QPU "Enable experimental direct VideoCore IV QPU backend" ON)
option(PIAI_VC4_REQUIRED "Refuse CPU inference when VC4 QPU execution is unavailable" OFF)

add_executable(piai
  src/main.cpp src/gguf_v3.cpp src/quant.cpp src/transformer.cpp src/api.cpp src/runtime.cpp
  src/platform.cpp src/platform/pi_models.cpp src/compute/dot.cpp src/compute/vc4_qpu.cpp
  src/compute/vc4_memory.cpp src/compute/vc4_runtime.cpp src/compute/vc4_compute.cpp
  src/inference/cache.cpp src/inference/rope.cpp src/inference/mode.cpp
  src/inference_v4.cpp src/inference_parallel.cpp src/architecture_v2.cpp)
target_include_directories(piai PRIVATE include)
target_compile_options(piai PRIVATE -Wall -Wextra -Wpedantic -O3 -ffast-math -funsafe-math-optimizations -fno-math-errno -fno-trapping-math -ftree-vectorize -ftree-slp-vectorize -fomit-frame-pointer -ffunction-sections -fdata-sections -pthread)

if(PIAI_NATIVE AND NOT CMAKE_CROSSCOMPILING)
  target_compile_options(piai PRIVATE -mcpu=native -mtune=native)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv6|armv6l|ARMv6|ARMv6l)$")
  target_compile_options(piai PRIVATE -mcpu=arm1176jzf-s -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(armv7|armv7l|ARMv7|ARMv7l)$")
  target_compile_options(piai PRIVATE -mcpu=cortex-a7 -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -marm)
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64|ARM64)$")
  target_compile_options(piai PRIVATE -march=armv8-a+simd)
endif()

if(PIAI_VC4_QPU)
  target_compile_definitions(piai PRIVATE PIAI_ENABLE_VC4_QPU=1)
endif()
if(PIAI_VC4_REQUIRED)
  target_compile_definitions(piai PRIVATE PIAI_VC4_REQUIRED=1)
endif()

target_link_options(piai PRIVATE -Wl,--gc-sections -pthread -Wl,--as-needed)
if(UNIX)
  target_link_libraries(piai PRIVATE pthread m)
endif()
install(TARGETS piai RUNTIME DESTINATION bin)
