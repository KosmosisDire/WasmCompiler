# my_aot_compiler_project/CMakeLists.txt
cmake_minimum_required(VERSION 3.23)
project(MyAOTLanguageProject CXX)

set(CMAKE_CXX_STANDARD 20)
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

# --- Wasmtime-cpp ---
FetchContent_Declare(
  wasmtime
  GIT_REPOSITORY https://github.com/bytecodealliance/wasmtime.git
  GIT_TAG        main
)

# --- Subdirectories ---
add_subdirectory(compiler) 
add_subdirectory(host_shared_lib)
add_subdirectory(native_host)

message(STATUS "Project Top-Level CMake Done.")