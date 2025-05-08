// native_host/src/main.cpp
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdexcept>
#include <optional>
#include <variant>
#include <string_view> // For std::string_view

#include "wasmtime.hh"
#include "host_shared_lib/functions.h"

// Function to read a file into a byte vector (assuming it's correct and unchanged)
std::vector<uint8_t> read_wasm_file(const std::string &path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open Wasm file: " + path);
    }
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(size));
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        throw std::runtime_error("Failed to read Wasm file: " + path);
    }
    return buffer;
}

int main(int argc, char *argv[])
{
    std::string wasm_file_path;
    if (argc < 2)
    {
        wasm_file_path = "compiler/output/application.wasm";
        std::cout << "No Wasm file path provided. Trying default: " << wasm_file_path << std::endl;
    }
    else
    {
        wasm_file_path = argv[1];
    }

    try
    {
        wasmtime::Engine engine;
        wasmtime::Store store(engine);

        auto wasm_bytes = read_wasm_file(wasm_file_path);

        wasmtime::Result<wasmtime::Module> module_compile_result = wasmtime::Module::compile(engine, wasm_bytes);
        if (!module_compile_result)
        {
            throw std::runtime_error("Failed to compile Wasm module: " + module_compile_result.err().message());
        }
        wasmtime::Module module = module_compile_result.unwrap();

        wasmtime::Linker linker(engine);

        wasmtime::Func wrapped_log_i32_func = wasmtime::Func::wrap(store.context(), shared_log_i32);

        // Linker::define takes Store::Context, string_view, string_view, const Extern&
        // Create an Extern from the Func.
        wasmtime::Extern extern_item = wrapped_log_i32_func;

        wasmtime::Result<std::monostate> define_result = linker.define(
            store.context(), // Store::Context cx
            "env",           // std::string_view module
            "log_i32",       // std::string_view name
            extern_item      // const Extern &item
        );
        if (!define_result)
        {
            // The error type for define is wasmtime::Error
            throw std::runtime_error("Failed to define import 'env.log_i32': " + define_result.err().message());
        }

        wasmtime::Result<wasmtime::Instance, wasmtime::TrapError> instance_result = linker.instantiate(store.context(), module);
        if (!instance_result)
        {
            throw std::runtime_error("Failed to instantiate Wasm module: " + instance_result.err().message());
        }
        wasmtime::Instance instance = instance_result.unwrap();

        std::optional<wasmtime::Extern> main_func_extern = instance.get(store.context(), "main");
        if (!main_func_extern)
        {
            throw std::runtime_error("Export 'main' not found in Wasm module.");
        }

        wasmtime::Func *func_ptr = std::get_if<wasmtime::Func>(&(*main_func_extern));
        if (!func_ptr)
        {
            throw std::runtime_error("Export 'main' is not a function.");
        }
        wasmtime::Func main_func = *func_ptr;

        std::vector<wasmtime::Val> main_args_vec;

        // Func::call returns Result<std::vector<Val>, TrapError>
        wasmtime::Result<std::vector<wasmtime::Val>, wasmtime::TrapError> call_trap_or_results = main_func.call(
            store.context(),
            main_args_vec);

        if (!call_trap_or_results)
        {
            throw std::runtime_error("Wasm 'main' function trapped: " + call_trap_or_results.err().message());
        }

        std::vector<wasmtime::Val> results_vec = call_trap_or_results.unwrap();

        if (results_vec.empty())
        {
            std::cout << "Wasm 'main' function returned no values." << std::endl;
        }
        else if (results_vec[0].kind() == wasmtime::ValKind::I32)
        {
            std::cout << "returned: " << results_vec[0].i32() << std::endl;
        }
        else
        {
            std::cout << "Wasm 'main' function returned an unexpected type for its first result." << std::endl;
        }
    }
    catch (const wasmtime::Error &e)
    {
        std::cerr << "Wasmtime error: " << e.message() << std::endl;
        return 1;
    }

    catch (const std::exception &e)
    {
        std::cerr << "Standard exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}