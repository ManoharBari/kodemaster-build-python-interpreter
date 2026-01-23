#pragma once

#include <vector>
#include <string>
#include "token.hpp"

class PyObject;
class NodeVisitor;

enum class AstNodeType
{
    Program,
    Block,
    Print,
    While,
    Break,
    Continue,
    Pass,
    If,
    Function,
    Return,
    Class,
    Property,
    UnaryOp,
    BinaryOp,
    Assign,
    Call,
    Name,
    String,
    Int,
    Float,
    Boolean,
    Null
};

class AstNode
{
public:
    AstNode(AstNodeType type) : type(type) {}
    virtual ~AstNode() = default;
    virtual PyObject *accept(NodeVisitor *visitor) = 0;
    AstNodeType type;
};

class IntNode : public AstNode
{
public:
    IntNode(Token value) : AstNode(AstNodeType::Int), value(value) {}
    PyObject *accept(NodeVisitor *visitor) override;
    Token value;
};

class FloatNode : public AstNode
{
public:
    FloatNode(Token value) : AstNode(AstNodeType::Float), value(value) {}
    PyObject *accept(NodeVisitor *visitor) override;
    Token value;
};

class StringNode : public AstNode
{
public:
    StringNode(Token value) : AstNode(AstNodeType::String), value(value) {}
    PyObject *accept(NodeVisitor *visitor) override;
    Token value;
};

class BooleanNode : public AstNode
{
public:
    BooleanNode(Token value) : AstNode(AstNodeType::Boolean), value(value) {}
    PyObject *accept(NodeVisitor *visitor) override;
    Token value;
};

class NullNode : public AstNode
{
public:
    NullNode() : AstNode(AstNodeType::Null) {}
    PyObject *accept(NodeVisitor *visitor) override;
};

class NameNode : public AstNode
{
public:
    NameNode(Token name) : AstNode(AstNodeType::Name), name(name) {}
    PyObject *accept(NodeVisitor *visitor) override;
    Token name;
};

class BinaryOpNode : public AstNode
{
public:
    BinaryOpNode(AstNode *left, Token op, AstNode *right)
        : AstNode(AstNodeType::BinaryOp), left(left), op(op), right(right) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *left;
    Token op;
    AstNode *right;
};

class UnaryOpNode : public AstNode
{
public:
    UnaryOpNode(Token op, AstNode *operand)
        : AstNode(AstNodeType::UnaryOp), op(op), operand(operand) {}
    PyObject *accept(NodeVisitor *visitor) override;
    Token op;
    AstNode *operand;
};

class AssignNode : public AstNode
{
public:
    AssignNode(Token name, AstNode *value)
        : AstNode(AstNodeType::Assign), name(name), value(value) {}
    PyObject *accept(NodeVisitor *visitor) override;
    Token name;
    AstNode *value;
};

class BlockNode : public AstNode
{
public:
    BlockNode(std::vector<AstNode *> statements)
        : AstNode(AstNodeType::Block), statements(statements) {}
    PyObject *accept(NodeVisitor *visitor) override;
    std::vector<AstNode *> statements;
};

class ProgramNode : public AstNode
{
public:
    ProgramNode(std::vector<AstNode *> statements)
        : AstNode(AstNodeType::Program), statements(statements) {}
    PyObject *accept(NodeVisitor *visitor) override;
    std::vector<AstNode *> statements;
};

class PrintNode : public AstNode
{
public:
    PrintNode(AstNode *expression)
        : AstNode(AstNodeType::Print), expression(expression) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *expression;
};

class NodeVisitor
{
public:
    virtual ~NodeVisitor() = default;
    virtual PyObject *visitProgramNode(ProgramNode *node) = 0;
    virtual PyObject *visitBlockNode(BlockNode *node) = 0;
    virtual PyObject *visitPrintNode(PrintNode *node) = 0;
    virtual PyObject *visitIntNode(IntNode *node) = 0;
    virtual PyObject *visitFloatNode(FloatNode *node) = 0;
    virtual PyObject *visitStringNode(StringNode *node) = 0;
    virtual PyObject *visitBooleanNode(BooleanNode *node) = 0;
    virtual PyObject *visitNullNode(NullNode *node) = 0;
    virtual PyObject *visitNameNode(NameNode *node) = 0;
    virtual PyObject *visitBinaryOpNode(BinaryOpNode *node) = 0;
    virtual PyObject *visitUnaryOpNode(UnaryOpNode *node) = 0;
    virtual PyObject *visitAssignNode(AssignNode *node) = 0;
};