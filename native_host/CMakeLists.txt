# my_aot_compiler_project/native_host/CMakeLists.txt
cmake_minimum_required(VERSION 3.23)
project(NativeHostInternal CXX)

set(BUILD_TESTS OFF CACHE BOOL "Build WABT tests" FORCE) 

FetchContent_MakeAvailable(wasmtime) 

add_executable(my_native_runner
    src/main.cpp
)

add_subdirectory(${wasmtime_SOURCE_DIR}/crates/c-api/ ${CMAKE_CURRENT_BINARY_DIR}/wasmtime)

target_link_libraries(my_native_runner PRIVATE
    wasmtime         
    host_shared_cpp_lib    
)

target_include_directories(my_native_runner PRIVATE
    wasmtime
)


message(STATUS "NativeHost CMake Done.")