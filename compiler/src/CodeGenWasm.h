// compiler/src/CodeGenWasm.h
#ifndef CODEGEN_WASM_H
#define CODEGEN_WASM_H

#include "AST.h" // Relative path assuming it's in the same dir or include path is set
#include <vector>
#include <cstdint> 

std::vector<uint8_t> generate_wasm_binary(const Node* ast_root, const std::string& module_name = "env", const std::string& log_func_name = "log_i32");

#endif // CODEGEN_WASM_H