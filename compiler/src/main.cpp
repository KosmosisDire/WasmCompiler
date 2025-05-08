// compiler/src/main.cpp
#include "AST.h"
#include "CodeGenWasm.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <vector>
#include <string> // Required for std::string

int main(int argc, char *argv[])
{
    std::string output_filename = "application.wasm";
    if (argc > 1)
    {
        output_filename = argv[1];
    }
    else
    {
        std::string default_output_path = "compiler/output/application.wasm";
        output_filename = default_output_path;
        std::cout << "No output filename provided, defaulting to: " << output_filename << std::endl;
    }

    std::unique_ptr<Node> ast_root = std::make_unique<Node>(
        OpType::ADD,
        std::make_unique<Node>(     // Left operand of outer ADD: This is the PrintNode
            std::make_unique<Node>( // The expression to be printed
                OpType::ADD,
                std::make_unique<Node>(10),
                std::make_unique<Node>(
                    OpType::MULTIPLY,
                    std::make_unique<Node>(2),
                    std::make_unique<Node>(5)))),
        std::make_unique<Node>(30));

    std::cout << "AST constructed: add(print(10 + (2 * 5)), 30)\n";
    std::cout << "Expected Wasm behavior: \n";
    std::cout << "  1. Prints '20' (via host import env.log_i32)\n";
    std::cout << "  2. The main Wasm function returns '50'\n";

    try
    {
        std::cout << "\n--- Generating WASM Binary (" << output_filename << ") ---\n";
        // We'll use "env" and "log_i32" as the external import names
        std::vector<uint8_t> wasm_bytes = generate_wasm_binary(ast_root.get(), "env", "log_i32");
        std::cout << "WASM binary generated (" << wasm_bytes.size() << " bytes)." << std::endl;

        std::ofstream wasm_file(output_filename, std::ios::binary | std::ios::trunc);
        if (wasm_file.is_open())
        {
            wasm_file.write(reinterpret_cast<const char *>(wasm_bytes.data()), wasm_bytes.size());
            wasm_file.close();
            std::cout << "WASM binary written to " << output_filename << std::endl;
        }
        else
        {
            std::cerr << "Error: Could not open " << output_filename << " for writing.\n";
            return 1;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error generating WASM binary: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}