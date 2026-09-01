CROSS ?= arm-none-eabi-
BUILD := build
COMMON := -ffreestanding -fno-builtin -fno-stack-protector -nostdlib -nostartfiles -nodefaultlibs -Wall -Wextra -O2 -Iinclude
CXX := $(CROSS)g++
CC := $(CROSS)gcc
BOOT_CPP := kernel/kernel.cpp kernel/uart.cpp kernel/shell.cpp kernel/ai_engine.cpp
.PHONY: all pi2 clean
all: pi2
pi2:
	mkdir -p $(BUILD)/pi2
	$(CC) $(COMMON) -mcpu=cortex-a7 -marm -c arch/arm32/start.S -o $(BUILD)/pi2/start.o
	$(CC) $(COMMON) -mcpu=cortex-a7 -marm -c arch/arm32/mmu.S -o $(BUILD)/pi2/mmu.o
	for f in $(BOOT_CPP); do $(CXX) $(COMMON) -fno-exceptions -fno-rtti -fno-use-cxa-atexit -mcpu=cortex-a7 -marm -c $$f -o $(BUILD)/pi2/$$(basename $$f .cpp).o; done
	$(CC) $(COMMON) -mcpu=cortex-a7 -marm -c kernel/memory.c -o $(BUILD)/pi2/memory.o
	$(CXX) -nostdlib -nostartfiles -nodefaultlibs -T arch/arm32/linker.ld $(BUILD)/pi2/start.o $(BUILD)/pi2/mmu.o $(BUILD)/pi2/kernel.o $(BUILD)/pi2/uart.o $(BUILD)/pi2/shell.o $(BUILD)/pi2/ai_engine.o $(BUILD)/pi2/memory.o -lgcc -o $(BUILD)/pi2/kernel.elf
	$(CROSS)objcopy -O binary $(BUILD)/pi2/kernel.elf $(BUILD)/pi2/kernel.img
clean:
	rm -rf $(BUILD)
