cmake_minimum_required(VERSION 3.15)
project(MyAOTCompilerInternal CXX)

# Make WABT available for this subproject
FetchContent_MakeAvailable(wabt) # This replaces Populate and add_subdirectory

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

install(TARGETS my_aot_compiler
    RUNTIME DESTINATION bin
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/output) 

message(STATUS "Compiler CMake Done.")