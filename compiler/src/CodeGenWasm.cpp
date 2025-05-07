// compiler/src/CodeGenWasm.cpp
#include "CodeGenWasm.h"
#include <sstream>
#include <stdexcept>
#include <iostream> 

// WABT Includes
#include "wabt/common.h"
#include "wabt/error-formatter.h"
#include "wabt/feature.h"
#include "wabt/ir.h"
#include "wabt/stream.h"
#include "wabt/wast-lexer.h"
#include "wabt/wast-parser.h"
#include "wabt/resolve-names.h"
#include "wabt/validator.h"
#include "wabt/binary-writer.h"


namespace { 

void generate_wat_expr_recursive(const Node* node, std::ostream& out, const std::string& log_func_name_in_wasm) {
    if (!node) throw std::runtime_error("Null node in AST during WAT generation");

    if (std::holds_alternative<NumberNode>(node->data)) {
        out << "    i32.const " << std::get<NumberNode>(node->data).value << "\n";
    } else if (std::holds_alternative<BinaryOpNode>(node->data)) {
        const auto& bin_op = std::get<BinaryOpNode>(node->data);
        generate_wat_expr_recursive(bin_op.left.get(), out, log_func_name_in_wasm);
        generate_wat_expr_recursive(bin_op.right.get(), out, log_func_name_in_wasm);
        switch (bin_op.op) {
            case OpType::ADD:      out << "    i32.add\n"; break;
            case OpType::MULTIPLY: out << "    i32.mul\n"; break;
            default: throw std::runtime_error("Unknown OpType in WAT generation");
        }
    } else if (std::holds_alternative<PrintNode>(node->data)) {
        const auto& print_op = std::get<PrintNode>(node->data);
        generate_wat_expr_recursive(print_op.expr.get(), out, log_func_name_in_wasm); 
        out << "    local.tee $temp_for_log\n"; 
        out << "    call $" << log_func_name_in_wasm << "\n"; // Use passed function name
        out << "    local.get $temp_for_log\n"; 
    }
    else {
        throw std::runtime_error("Unknown AST node type in WAT generation");
    }
}

std::string generate_full_wat_module_string(const Node* ast_root, 
                                            const std::string& import_module_name, 
                                            const std::string& import_func_name,
                                            const std::string& log_func_name_in_wasm) {
    std::stringstream ss;
    ss << "(module\n";
    // Use passed names for import
    ss << "  (import \"" << import_module_name << "\" \"" << import_func_name << "\" (func $" << log_func_name_in_wasm << " (param i32))) \n";
    ss << "  (func $calculate (result i32) (local $temp_for_log i32)\n";
    generate_wat_expr_recursive(ast_root, ss, log_func_name_in_wasm);
    ss << "  )\n";
    ss << "  (export \"calculate\" (func $calculate))\n";
    ss << "  (func (export \"main\") (result i32)\n"; // 'main' is a common convention for wasmtime CLI
    ss << "    call $calculate\n";
    ss << "  )\n";
    ss << ")\n";
    return ss.str();
}

} // namespace

std::vector<uint8_t> generate_wasm_binary(const Node* ast_root, const std::string& module_name, const std::string& log_func_name) {
    // The name we use inside Wasm for the imported log function can be simpler, e.g., just "log_i32_impl"
    // This decouples it slightly from the external import name.
    std::string log_func_name_in_wasm = "imported_log_i32"; 
    std::string wat_string = generate_full_wat_module_string(ast_root, module_name, log_func_name, log_func_name_in_wasm);

    // std::cout << "--- Generated WAT ---\n" << wat_string << "---------------------\n"; // For debugging

    wabt::Errors errors;
    wabt::Features features; 
    features.EnableAll(); 

    std::unique_ptr<wabt::WastLexer> lexer = wabt::WastLexer::CreateBufferLexer("in-memory.wat", wat_string.c_str(), wat_string.size(), &errors);
    if (!lexer) { 
        throw std::runtime_error("Failed to create WABT WastLexer.");
    }
    if (errors.size() > 0) { 
        throw std::runtime_error("WABT WastLexer creation reported errors: " + wabt::FormatErrorsToString(errors, wabt::Location::Type::Text));
    }

    std::unique_ptr<wabt::Module> ir_module;
    wabt::WastParseOptions parse_options(features);
    wabt::Result result = wabt::ParseWatModule(lexer.get(), &ir_module, &errors, &parse_options);

    if (wabt::Failed(result)) {
        std::string err_msg = "WABT ParseWatModule failed: ";
        err_msg += wabt::FormatErrorsToString(errors, wabt::Location::Type::Text);
        throw std::runtime_error(err_msg);
    }

    result = wabt::ResolveNamesModule(ir_module.get(), &errors);
    if (wabt::Failed(result)) {
        std::string err_msg = "WABT ResolveNamesModule failed: ";
        err_msg += wabt::FormatErrorsToString(errors, wabt::Location::Type::Text);
        throw std::runtime_error(err_msg);
    }

    wabt::ValidateOptions val_options(features);
    result = wabt::ValidateModule(ir_module.get(), &errors, val_options);
    if (wabt::Failed(result)) {
        std::string err_msg = "WABT ValidateModule failed: ";
        err_msg += wabt::FormatErrorsToString(errors, wabt::Location::Type::Text);
        throw std::runtime_error(err_msg);
    }

    wabt::MemoryStream memory_stream;
    wabt::WriteBinaryOptions write_options;
    write_options.relocatable = false; 

    result = wabt::WriteBinaryModule(&memory_stream, ir_module.get(), write_options);
    if (wabt::Failed(result)) {
        throw std::runtime_error("WABT WriteBinaryModule failed.");
    }
    
    return memory_stream.output_buffer().data;
}