CROSS ?= arm-none-eabi-
AARCH64_CROSS ?= aarch64-none-elf-
BUILD := build
CFLAGS := -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -nostartfiles -nodefaultlibs -Wall -Wextra -O2 -Iinclude
CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti -fno-use-cxa-atexit
KERNEL_CPP := kernel/kernel.cpp kernel/console.cpp kernel/printf.cpp kernel/panic.cpp kernel/ai_engine.cpp kernel/scheduler.cpp kernel/syscall.cpp kernel/vfs.cpp kernel/timer.cpp kernel/mmu.cpp ai/tensor.cpp ai/gguf.cpp ai/tokenizer.cpp
KERNEL_C := kernel/memory.c

.PHONY: all armv6 armv7 armv8 clean
all: armv6 armv7 armv8

armv6:
	mkdir -p $(BUILD)/armv6
	$(CROSS)gcc $(CFLAGS) -mcpu=arm1176jzf-s -marm -c arch/arm32/start.S -o $(BUILD)/armv6/start.o
	$(CROSS)gcc $(CFLAGS) -mcpu=arm1176jzf-s -marm -c arch/arm32/vectors.S -o $(BUILD)/armv6/vectors.o
	$(CROSS)gcc $(CFLAGS) -mcpu=arm1176jzf-s -marm -c arch/arm32/mmu.S -o $(BUILD)/armv6/mmu.o
	for f in $(KERNEL_CPP); do $(CROSS)g++ $(CXXFLAGS) -mcpu=arm1176jzf-s -marm -c $$f -o $(BUILD)/armv6/$$(basename $$f .cpp).o; done
	$(CROSS)gcc $(CFLAGS) -mcpu=arm1176jzf-s -marm -c $(KERNEL_C) -o $(BUILD)/armv6/memory.o
	$(CROSS)g++ -nostdlib -T arch/arm32/linker.ld $(BUILD)/armv6/*.o -o $(BUILD)/armv6/kernel.elf
	$(CROSS)objcopy -O binary $(BUILD)/armv6/kernel.elf $(BUILD)/armv6/kernel.img

armv7:
	mkdir -p $(BUILD)/armv7
	$(CROSS)gcc $(CFLAGS) -mcpu=cortex-a7 -marm -c arch/arm32/start.S -o $(BUILD)/armv7/start.o
	$(CROSS)gcc $(CFLAGS) -mcpu=cortex-a7 -marm -c arch/arm32/vectors.S -o $(BUILD)/armv7/vectors.o
	$(CROSS)gcc $(CFLAGS) -mcpu=cortex-a7 -marm -c arch/arm32/mmu.S -o $(BUILD)/armv7/mmu.o
	for f in $(KERNEL_CPP); do $(CROSS)g++ $(CXXFLAGS) -mcpu=cortex-a7 -marm -c $$f -o $(BUILD)/armv7/$$(basename $$f .cpp).o; done
	$(CROSS)gcc $(CFLAGS) -mcpu=cortex-a7 -marm -c $(KERNEL_C) -o $(BUILD)/armv7/memory.o
	$(CROSS)g++ -nostdlib -T arch/arm32/linker.ld $(BUILD)/armv7/*.o -o $(BUILD)/armv7/kernel.elf
	$(CROSS)objcopy -O binary $(BUILD)/armv7/kernel.elf $(BUILD)/armv7/kernel.img

armv8:
	mkdir -p $(BUILD)/armv8
	$(AARCH64_CROSS)gcc $(CFLAGS) -march=armv8-a -c arch/arm64/start.S -o $(BUILD)/armv8/start.o
	for f in $(KERNEL_CPP); do $(AARCH64_CROSS)g++ $(CXXFLAGS) -march=armv8-a -c $$f -o $(BUILD)/armv8/$$(basename $$f .cpp).o; done
	$(AARCH64_CROSS)gcc $(CFLAGS) -march=armv8-a -c $(KERNEL_C) -o $(BUILD)/armv8/memory.o
	$(AARCH64_CROSS)g++ -nostdlib -T arch/arm64/linker.ld $(BUILD)/armv8/*.o -o $(BUILD)/armv8/kernel.elf
	$(AARCH64_CROSS)objcopy -O binary $(BUILD)/armv8/kernel.elf $(BUILD)/armv8/kernel8.img

clean:
	rm -rf $(BUILD)
