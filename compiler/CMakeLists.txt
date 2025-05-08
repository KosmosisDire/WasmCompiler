# my_aot_compiler_project/compiler/CMakeLists.txt
cmake_minimum_required(VERSION 3.23)
project(MyAOTCompilerInternal CXX) 

FetchContent_MakeAvailable(wabt)

add_executable(my_aot_compiler
    src/main.cpp
    src/AST.h
    src/CodeGenWasm.h
    src/CodeGenWasm.cpp
)

target_include_directories(my_aot_compiler PRIVATE
    src 
    ${wabt_SOURCE_DIR}/include   
    ${wabt_BINARY_DIR}          
)

target_link_libraries(my_aot_compiler PRIVATE
    wabt 
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output) 

message(STATUS "Compiler CMake Done.")