cmake_minimum_required(VERSION 3.23)
project(HostSharedLibInternal CXX)

add_library(host_shared_cpp_lib STATIC 
    src/functions.cpp
)

target_include_directories(host_shared_cpp_lib 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include> # For when/if you install this library
)

message(STATUS "HostSharedLib CMake Done.")