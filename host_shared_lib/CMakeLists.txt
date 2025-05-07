cmake_minimum_required(VERSION 3.15)
project(HostSharedLibInternal CXX)

add_library(host_shared_cpp_lib STATIC 
    src/functions.cpp
)

target_include_directories(host_shared_cpp_lib 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include> # For when/if you install this library
)

# Optional: Install the library (headers and static lib)
# install(TARGETS host_shared_cpp_lib ARCHIVE DESTINATION lib)
# install(DIRECTORY include/ DESTINATION include)

message(STATUS "HostSharedLib CMake Done.")