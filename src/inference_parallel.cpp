// The complete inference implementation lives in inference_v4.cpp.
// Keep this translation unit as a single inclusion point so CMake does not
// compile the implementation twice or rename its internal static symbols.
// Parallel execution is implemented inside the inference implementation;
// this file intentionally contains no second copy of mv().
#include "inference_v4.cpp"
