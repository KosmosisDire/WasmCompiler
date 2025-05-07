cmake_minimum_required(VERSION 3.15)
project(MyAOTLanguageProject CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# --- WABT (WebAssembly Binary Toolkit) ---
FetchContent_Declare(
  wabt
  GIT_REPOSITORY https://github.com/WebAssembly/wabt.git
  GIT_TAG        1.0.33 
)
set(WABT_BUILD_TOOLS OFF CACHE BOOL "Build WABT command-line tools" FORCE)
set(WABT_BUILD_TESTS OFF CACHE BOOL "Build WABT tests" FORCE)

# --- Wasmtime (Main Repository, includes C++ API) ---
FetchContent_Declare(
  wasmtime_sdk 
  GIT_REPOSITORY https://github.com/bytecodealliance/wasmtime.git
  GIT_TAG        v32.0.0 # Using your suggested tag for the main Wasmtime repository
  CMAKE_ARGS # Arguments passed to Wasmtime's CMake configure step
    -DWASMTIME_C_API=ON       # Ensure C API is built (dependency for C++ API)
    -DWASMTIME_CPP_API=ON   # Ensure C++ API is built
    -DWASMTIME_EXAMPLES=OFF   # Optional: disable examples to speed up build
    -DWASMTIME_TESTS=OFF      # Optional: disable tests
    # -DCMAKE_BUILD_TYPE=Release # Optional: Wasmtime might build faster in Release
)

# --- Subdirectories ---
add_subdirectory(compiler)
add_subdirectory(host_shared_lib)
add_subdirectory(native_host)

message(STATUS "Project Top-Level CMake Done.")