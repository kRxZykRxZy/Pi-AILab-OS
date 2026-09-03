#pragma once
#include <cstddef>
#include <cstdint>

namespace piai::compute::vc4_kernel {
// Hand-encoded VideoCore IV QPU program. It follows the documented 64-bit
// instruction layout and uses the VPM DMA path:
//   uniform[0] -> input (two contiguous 16-float rows)
//   uniform[1] -> output (one 16-float row)
// The first row and second row are added lane-wise and written back.
//
// The program is deliberately kept as raw QPU words so the project has no
// Python or assembler dependency at runtime or build time.
inline constexpr uint64_t vector_add[] = {
    0xe0024c6783021000ULL,
    0x10024ca715820d80ULL,
    0x100249e715cb2d80ULL,
    0xe0024c6700201000ULL,
    0xe0024c6700001a00ULL,
    0x1002402715c30d80ULL,
    0x1002406715c30d80ULL,
    0x10024c27019e7040ULL,
    0xe0024c6780904000ULL,
    0x10024ca715820d80ULL,
    0x100249e715cb2d80ULL,
    0x30024027159e7000ULL,
    0x10024027159e7000ULL,
    0x100249e7009e7000ULL,
    0x100249e7009e7000ULL
};
inline constexpr std::size_t vector_add_size = sizeof(vector_add);
}
