cmake_minimum_required(VERSION 3.15)
project(NativeHostInternal CXX)

# Make Wasmtime SDK available (which includes the C++ API)
FetchContent_MakeAvailable(wasmtime_sdk) # This replaces Populate and add_subdirectory

add_executable(my_native_runner
    src/main.cpp
)

target_link_libraries(my_native_runner PRIVATE
    wasmtime::cpp          # This target should be provided by the main Wasmtime build
    host_shared_cpp_lib    
)

# Include directories from host_shared_lib should be handled by its PUBLIC target_include_directories

install(TARGETS my_native_runner RUNTIME DESTINATION bin)

message(STATUS "NativeHost CMake Done.")