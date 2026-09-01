CROSS ?= arm-none-eabi-
AARCH64_CROSS ?= aarch64-none-elf-
BUILD := build
COMMON_C := -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -nostartfiles -nodefaultlibs -Wall -Wextra -O2 -Iinclude
COMMON_CPP := $(COMMON_C) -fno-exceptions -fno-rtti -fno-use-cxa-atexit
ARM32_C := $(CROSS)gcc
ARM32_CXX := $(CROSS)g++
ARM64_C := $(AARCH64_CROSS)gcc
ARM64_CXX := $(AARCH64_CROSS)g++

.PHONY: all armv6 armv7 armv8 clean
all: armv6 armv7 armv8

armv6:
	mkdir -p $(BUILD)/armv6
	$(ARM32_C) $(COMMON_C) -DPILAB_ARMV6 -mcpu=arm1176jzf-s -marm -c arch/arm32/start.S -o $(BUILD)/armv6/start.o
	$(ARM32_C) $(COMMON_C) -DPILAB_ARMV6 -mcpu=arm1176jzf-s -marm -c arch/arm32/vectors.S -o $(BUILD)/armv6/vectors.o
	$(ARM32_CXX) $(COMMON_CPP) -DPILAB_ARMV6 -mcpu=arm1176jzf-s -marm -c kernel/kernel.cpp -o $(BUILD)/armv6/kernel.o
	$(ARM32_CXX) $(COMMON_CPP) -DPILAB_ARMV6 -mcpu=arm1176jzf-s -marm -c kernel/console.cpp -o $(BUILD)/armv6/console.o
	$(ARM32_C) $(COMMON_C) -DPILAB_ARMV6 -mcpu=arm1176jzf-s -marm -c kernel/memory.c -o $(BUILD)/armv6/memory.o
	$(ARM32_CXX) -nostdlib -T arch/arm32/linker.ld $(BUILD)/armv6/start.o $(BUILD)/armv6/vectors.o $(BUILD)/armv6/kernel.o $(BUILD)/armv6/console.o $(BUILD)/armv6/memory.o -o $(BUILD)/armv6/kernel.elf
	$(CROSS)objcopy -O binary $(BUILD)/armv6/kernel.elf $(BUILD)/armv6/kernel.img

armv7:
	mkdir -p $(BUILD)/armv7
	$(ARM32_C) $(COMMON_C) -DPILAB_ARMV7 -mcpu=cortex-a7 -marm -c arch/arm32/start.S -o $(BUILD)/armv7/start.o
	$(ARM32_C) $(COMMON_C) -DPILAB_ARMV7 -mcpu=cortex-a7 -marm -c arch/arm32/vectors.S -o $(BUILD)/armv7/vectors.o
	$(ARM32_CXX) $(COMMON_CPP) -DPILAB_ARMV7 -mcpu=cortex-a7 -marm -c kernel/kernel.cpp -o $(BUILD)/armv7/kernel.o
	$(ARM32_CXX) $(COMMON_CPP) -DPILAB_ARMV7 -mcpu=cortex-a7 -marm -c kernel/console.cpp -o $(BUILD)/armv7/console.o
	$(ARM32_C) $(COMMON_C) -DPILAB_ARMV7 -mcpu=cortex-a7 -marm -c kernel/memory.c -o $(BUILD)/armv7/memory.o
	$(ARM32_CXX) -nostdlib -T arch/arm32/linker.ld $(BUILD)/armv7/start.o $(BUILD)/armv7/vectors.o $(BUILD)/armv7/kernel.o $(BUILD)/armv7/console.o $(BUILD)/armv7/memory.o -o $(BUILD)/armv7/kernel.elf
	$(CROSS)objcopy -O binary $(BUILD)/armv7/kernel.elf $(BUILD)/armv7/kernel.img

armv8:
	mkdir -p $(BUILD)/armv8
	$(ARM64_C) $(COMMON_C) -DPILAB_ARMV8 -march=armv8-a -c arch/arm64/start.S -o $(BUILD)/armv8/start.o
	$(ARM64_CXX) $(COMMON_CPP) -DPILAB_ARMV8 -march=armv8-a -c kernel/kernel.cpp -o $(BUILD)/armv8/kernel.o
	$(ARM64_CXX) $(COMMON_CPP) -DPILAB_ARMV8 -march=armv8-a -c kernel/console.cpp -o $(BUILD)/armv8/console.o
	$(ARM64_C) $(COMMON_C) -DPILAB_ARMV8 -march=armv8-a -c kernel/memory.c -o $(BUILD)/armv8/memory.o
	$(ARM64_CXX) -nostdlib -T arch/arm64/linker.ld $(BUILD)/armv8/start.o $(BUILD)/armv8/kernel.o $(BUILD)/armv8/console.o $(BUILD)/armv8/memory.o -o $(BUILD)/armv8/kernel.elf
	$(AARCH64_CROSS)objcopy -O binary $(BUILD)/armv8/kernel.elf $(BUILD)/armv8/kernel8.img

clean:
	rm -rf $(BUILD)
