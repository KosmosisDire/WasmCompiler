// native_host/src/main.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <array> // For std::array

#include "wasmtime.hpp" // Wasmtime C++ API
#include "host_shared_lib/functions.h" // Our shared library functions

// Function to read a file into a byte vector
std::vector<uint8_t> read_wasm_file(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open Wasm file: " + path);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size)); // Ensure size_t for vector constructor
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read Wasm file: " + path);
    }
    return buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_application.wasm>" << std::endl;
        // For testing, let's try a default path if no arg is given
        // This assumes application.wasm is in compiler/output relative to project root
        // and my_native_runner is run from build/ or project root.
        // This is still a bit fragile.
        std::string default_wasm_path;
        #ifdef _WIN32
        default_wasm_path = "..\\compiler\\output\\application.wasm"; // If running from build/bin/
        if (argc == 1) { // Check if my_native_runner itself is in build/bin
             // A common case when running from IDE or directly from build dir
             // If build dir is sibling to compiler dir:
             // default_wasm_path = "../compiler/output/application.wasm";
             // If build dir is my_aot_compiler_project/build:
             default_wasm_path = "../compiler/output/application.wasm"; 
        }
        #else
        default_wasm_path = "../compiler/output/application.wasm"; // If running from build/bin/
        #endif
        
        // A more robust default might be if CMAKE_SOURCE_DIR is passed as a define
        // For now, this is a guess.
        std::cout << "No Wasm file path provided. Trying default: " << default_wasm_path << std::endl;
        // We'll use default_wasm_path if argc < 2
    }
    
    std::string wasm_file_path = (argc < 2) ? "../compiler/output/application.wasm" : argv[1];
     // A common default if building in 'build' dir and compiler output is 'compiler/output'
    if (argc < 2) {
        std::cout << "No wasm file provided, attempting to load from default path: " << wasm_file_path << std::endl;
    }


    try {
        wasmtime::Engine engine;
        wasmtime::Store store(engine);

        std::cout << "Loading Wasm module from: " << wasm_file_path << std::endl;
        auto wasm_bytes = read_wasm_file(wasm_file_path);
        // Use Result for better error handling from wasmtime-cpp
        auto module_result = wasmtime::Module::validate(engine, wasm_bytes);
        if (!module_result) {
             throw std::runtime_error("Failed to validate Wasm module: " + module_result.error().message());
        }
        wasmtime::Module module = module_result.value();


        wasmtime::Linker linker(engine);

        // Define the "env.log_i32" import using shared_log_i32
        // The Wasm module imports "env"."log_i32"
        // We provide our C++ function shared_log_i32 (from host_shared_lib) as its implementation.
        // The signature in Wasm is (param i32) -> nil.
        // shared_log_i32 is void(int32_t), which matches.
        auto define_result = linker.define(
            store,
            "env",           // Wasm import module name
            "log_i32",       // Wasm import function name
            wasmtime::Func::wrap(store, shared_log_i32) // Wrap the C++ function
        );
        if (!define_result) {
            throw std::runtime_error("Failed to define import 'env.log_i32': " + define_result.error().message());
        }

        std::cout << "Instantiating module..." << std::endl;
        auto instance_result = linker.instantiate(store, module);
        if (!instance_result) {
            throw std::runtime_error("Failed to instantiate Wasm module: " + instance_result.error().message());
        }
        wasmtime::Instance instance = instance_result.value();

        std::cout << "Calling exported 'main' function..." << std::endl;
        auto main_func_obj = instance.get(store, "main");
        if (!main_func_obj || !main_func_obj->func()) {
            throw std::runtime_error("Failed to find exported 'main' function in Wasm module.");
        }
        wasmtime::Func main_func = main_func_obj->func().value();
        
        std::array<wasmtime::Val, 0> args; 
        std::array<wasmtime::Val, 1> results; 

        auto trap_result = main_func.call(store, args, results);
        if (trap_result) { // A trap occurred
            throw std::runtime_error("Wasm trapped: " + trap_result.message());
        }

        if (results[0].kind() == wasmtime::ValKind::I32) {
            std::cout << "Wasm 'main' function returned: " << results[0].i32() << std::endl;
        } else {
            std::cout << "Wasm 'main' function returned an unexpected type." << std::endl;
        }

    } catch (const wasmtime::Error& e) { 
        std::cerr << "Wasmtime error: " << e.message();
        // If it's a trap, wasmtime::Error might contain more info.
        // For wasmtime-cpp v19+, you might need to inspect the error type
        // or use specific methods if available to get trap details.
        // The basic message() usually suffices.
        std::cerr << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}