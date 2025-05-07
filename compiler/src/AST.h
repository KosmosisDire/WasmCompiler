// compiler/src/AST.h
#ifndef AST_H
#define AST_H

#include <memory>
#include <variant>
#include <string> 

struct Node; // Forward declaration

struct NumberNode {
    int value;
    explicit NumberNode(int val) : value(val) {}
};

enum class OpType {
    ADD,
    MULTIPLY
};

struct BinaryOpNode {
    OpType op;
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;

    BinaryOpNode(OpType o, std::unique_ptr<Node> l, std::unique_ptr<Node> r)
        : op(o), left(std::move(l)), right(std::move(r)) {}
};

struct PrintNode {
    std::unique_ptr<Node> expr;

    explicit PrintNode(std::unique_ptr<Node> e) : expr(std::move(e)) {}
};

struct Node {
    std::variant<NumberNode, BinaryOpNode, PrintNode> data;

    // Constructor for NumberNode
    Node(int value) : data(NumberNode(value)) {}

    // Constructor for BinaryOpNode
    Node(OpType op, std::unique_ptr<Node> left, std::unique_ptr<Node> right)
        : data(BinaryOpNode(op, std::move(left), std::move(right))) {}

    // Constructor for PrintNode
    explicit Node(std::unique_ptr<Node> expr_to_print) 
        : data(PrintNode(std::move(expr_to_print))) {}
};

#endif // AST_H