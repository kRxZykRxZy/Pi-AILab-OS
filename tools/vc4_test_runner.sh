#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD="$ROOT/build-vc4-test"
GEN="$BUILD/generated"
mkdir -p "$GEN"

if ! python3 "$ROOT/tools/vc4_qpu_kernels.py" "$GEN/vc4_kernels.hpp"; then
    echo "ERROR: VC4 kernel generation failed." >&2
    exit 1
fi

if [ ! -s "$GEN/vc4_kernels.hpp" ]; then
    echo "ERROR: no generated VC4 kernel header." >&2
    exit 1
fi

echo "Building VC4 QPU smoke test..."
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic \
    -I"$ROOT/include" -I"$GEN" \
    "$ROOT/tools/vc4_smoke_test.cpp" \
    "$ROOT/src/compute/vc4_compute.cpp" \
    "$ROOT/src/compute/vc4_runtime.cpp" \
    "$ROOT/src/compute/vc4_memory.cpp" \
    -o "$BUILD/piai-vc4-smoke" -pthread

echo "Running VC4 QPU smoke test..."
exec "$BUILD/piai-vc4-smoke"
