# Pi AI Lab OS

Pi AI Lab OS is a lightweight, native operating system for Raspberry Pi-class ARM hardware. V0.1 targets ARMv6, ARMv7-A and ARMv8-A, with a common C/C++ kernel core and architecture-specific boot code.

## V0.1 goals

- Bare-metal kernel (no Linux/Python dependency)
- ARMv6, ARMv7-A and ARMv8-A build targets
- Exception/vector setup
- MMIO UART console
- Physical memory allocator
- Kernel heap
- Timer/interrupt abstraction
- Process/thread and syscall foundation
- Virtual filesystem abstraction
- Native AI execution service boundary
- Native HTTP/API service boundary
- Model storage under `/models`
- Cross-compilation from a normal Linux build host

The AI runtime is deliberately a user-space/native component rather than kernel code. The kernel provides memory, scheduling, files, timers and device primitives; the AI engine consumes those services.

## Important

This is a real bare-metal OS project, not a Linux distribution. ARMv6/v7 and ARMv8 use different execution modes and page-table/exception mechanisms, so they have separate architecture implementations behind one common kernel API.
