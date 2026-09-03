#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
BUILD="$ROOT/build-vc4-test"
GEN="$BUILD/generated"
mkdir -p "$GEN"
python3 "$ROOT/tools/vc4_qpu_kernels.py" "$GEN/vc4_kernels.hpp"
g++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -I"$ROOT/include" -I"$GEN" \
  "$ROOT/tools/vc4_smoke_test.cpp" "$ROOT/src/compute/vc4_compute.cpp" \
  "$ROOT/src/compute/vc4_runtime.cpp" "$ROOT/src/compute/vc4_memory.cpp" \
  -o "$BUILD/piai-vc4-smoke" -pthread
exec "$BUILD/piai-vc4-smoke"
