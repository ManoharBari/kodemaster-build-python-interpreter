#pragma once

#include <vector>
#include <string>
#include <memory>
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

class PassNode : public AstNode
{
public:
    PassNode() : AstNode(AstNodeType::Pass) {}
    PyObject *accept(NodeVisitor *visitor) override;
};

class BreakNode : public AstNode
{
public:
    BreakNode() : AstNode(AstNodeType::Break) {}
    PyObject *accept(NodeVisitor *visitor) override;
};

class ContinueNode : public AstNode
{
public:
    ContinueNode() : AstNode(AstNodeType::Continue) {}
    PyObject *accept(NodeVisitor *visitor) override;
};

class ReturnNode : public AstNode
{
public:
    ReturnNode(AstNode *value) : AstNode(AstNodeType::Return), value(value) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *value;
};

class IfNode : public AstNode
{
public:
    IfNode(AstNode *condition, AstNode *thenBranch,
           std::vector<std::pair<AstNode *, AstNode *>> elifBranches,
           AstNode *elseBranch)
        : AstNode(AstNodeType::If), condition(condition),
          thenBranch(thenBranch), elifBranches(elifBranches),
          elseBranch(elseBranch) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *condition;
    AstNode *thenBranch;
    std::vector<std::pair<AstNode *, AstNode *>> elifBranches;
    AstNode *elseBranch;
};

class WhileNode : public AstNode
{
public:
    WhileNode(AstNode *condition, AstNode *body)
        : AstNode(AstNodeType::While), condition(condition), body(body) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *condition;
    AstNode *body;
};

class FunctionNode : public AstNode
{
public:
    FunctionNode(const std::string &name, std::vector<std::string> params, AstNode *body)
        : AstNode(AstNodeType::Function), name(name), params(params), body(body) {}
    PyObject *accept(NodeVisitor *visitor) override;
    std::string name;
    std::vector<std::string> params;
    AstNode *body;
};

class CallNode : public AstNode
{
public:
    CallNode(AstNode *callee, std::vector<AstNode *> args)
        : AstNode(AstNodeType::Call), callee(callee), args(args) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *callee;
    std::vector<AstNode *> args;
};

class PropertyNode : public AstNode
{
public:
    PropertyNode(AstNode *object, const std::string &property)
        : AstNode(AstNodeType::Property), object(object), property(property) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *object;
    std::string property;
};

class ClassNode : public AstNode
{
public:
    ClassNode(const std::string &name, AstNode *body)
        : AstNode(AstNodeType::Class), name(name), body(body) {}
    PyObject *accept(NodeVisitor *visitor) override;
    std::string name;
    AstNode *body;
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

class PropertyAssignNode : public AstNode
{
public:
    PropertyAssignNode(AstNode *object, const std::string &property, AstNode *value)
        : AstNode(AstNodeType::Assign), object(object), property(property), value(value) {}
    PyObject *accept(NodeVisitor *visitor) override;
    AstNode *object;
    std::string property;
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

// ==================== If/Elif/Else ====================
struct ElifClause
{
    std::shared_ptr<AstNode> condition;
    std::shared_ptr<AstNode> body;
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
    virtual PyObject *visitPassNode(PassNode *node) = 0;
    virtual PyObject *visitBreakNode(BreakNode *node) = 0;
    virtual PyObject *visitContinueNode(ContinueNode *node) = 0;
    virtual PyObject *visitReturnNode(ReturnNode *node) = 0;
    virtual PyObject *visitIfNode(IfNode *node) = 0;
    virtual PyObject *visitWhileNode(WhileNode *node) = 0;
    virtual PyObject *visitFunctionNode(FunctionNode *node) = 0;
    virtual PyObject *visitCallNode(CallNode *node) = 0;
    virtual PyObject *visitPropertyNode(PropertyNode *node) = 0;
    virtual PyObject *visitClassNode(ClassNode *node) = 0;
    virtual PyObject *visitIntNode(IntNode *node) = 0;
    virtual PyObject *visitFloatNode(FloatNode *node) = 0;
    virtual PyObject *visitStringNode(StringNode *node) = 0;
    virtual PyObject *visitBooleanNode(BooleanNode *node) = 0;
    virtual PyObject *visitNullNode(NullNode *node) = 0;
    virtual PyObject *visitNameNode(NameNode *node) = 0;
    virtual PyObject *visitBinaryOpNode(BinaryOpNode *node) = 0;
    virtual PyObject *visitUnaryOpNode(UnaryOpNode *node) = 0;
    virtual PyObject *visitAssignNode(AssignNode *node) = 0;
    virtual PyObject *visitPropertyAssignNode(PropertyAssignNode *node) = 0;
};