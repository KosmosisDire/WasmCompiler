// host_shared_lib/include/host_shared_lib/functions.h
#ifndef HOST_SHARED_FUNCTIONS_H
#define HOST_SHARED_FUNCTIONS_H

#include <cstdint> // For int32_t

// This function will be called by the Wasm module (via the host)
// It needs C linkage if it's to be easily found by Emscripten later without name mangling,
// or if you were dynamically loading it in C. For static linking in C++ and Wasmtime's
// C++ API, C linkage isn't strictly necessary but is good practice for cross-language calls.
#ifdef __cplusplus
extern "C" {
#endif

void shared_log_i32(int32_t value);

// Add declarations for other shared functions here in the future

#ifdef __cplusplus
} // extern "C"
#endif

#endif // HOST_SHARED_FUNCTIONS_H