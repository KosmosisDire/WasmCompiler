// host_shared_lib/src/functions.cpp
#include "host_shared_lib/functions.h" // Correct path based on include_directories

#ifdef __EMSCRIPTEN__
#include <emscripten.h> // Will be used when compiling with Emscripten
#else
#include <iostream>     // For native builds
#endif

// Definition of the shared function
void shared_log_i32(int32_t value) {
#ifdef __EMSCRIPTEN__
    // When compiled with Emscripten for the web host
    // EM_ASM_ is a way to inline JavaScript
    EM_ASM_({
        console.log($0);
    }, value);
#else
    // When compiled natively for the native_host
    std::cout << value << std::endl;
#endif
}