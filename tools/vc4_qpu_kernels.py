#!/usr/bin/env python3
"""Build the small custom VC4 QPU kernel pack used by the native backend.

The assembler is a build-time dependency only. The generated header contains
machine code, so the C++ runtime has no Python/OpenCL dependency.
"""
import pathlib
import sys

out = pathlib.Path(sys.argv[1])
out.parent.mkdir(parents=True, exist_ok=True)

try:
    from videocore.assembler import qpu, assemble
except Exception as exc:
    out.write_text(
        '#pragma once\n'
        'namespace piai::compute::vc4_kernel {\n'
        'static const unsigned char vector_add[] = {};\n'
        'static const unsigned vector_add_size = 0;\n'
        '}\n', encoding='utf-8')
    print('VC4 QPU assembler unavailable; generated disabled kernel pack:', exc)
    raise SystemExit(0)

@qpu
def vector_add(asm):
    # uniforms[0] = input A, uniforms[1] = input B, uniforms[2] = output C.
    # Each QPU processes one 16-float vector. The host launches up to all 12
    # VC4 QPUs and gives each thread a different pair of bus addresses.
    setup_dma_load(nrows=2)
    start_dma_load(uniform)
    wait_dma_load()
    setup_vpm_read(nrows=2)
    setup_vpm_write()
    mov(r0, vpm)
    mov(r1, vpm)
    fadd(vpm, r0, r1)
    setup_dma_store(nrows=1)
    start_dma_store(uniform)
    wait_dma_store()
    exit()

code = bytes(assemble(vector_add))
with out.open('w', encoding='utf-8') as f:
    f.write('#pragma once\n#include <cstddef>\n#include <cstdint>\n')
    f.write('namespace piai::compute::vc4_kernel {\n')
    f.write('static const uint8_t vector_add[] = {')
    for i, b in enumerate(code):
        if i % 16 == 0:
            f.write('\n  ')
        f.write(f'0x{b:02x}, ')
    f.write('\n};\n')
    f.write(f'static constexpr size_t vector_add_size = {len(code)};\n')
    f.write('}\n')
