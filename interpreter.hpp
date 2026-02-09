#pragma once

#include "ast.hpp"
#include "pyobject.hpp"
#include "scope.hpp"
#include <memory>

class Interpreter : public NodeVisitor
{
public:
    Interpreter();
    void interpret(ProgramNode *program);

    PyObject *visitProgramNode(ProgramNode *node) override;
    PyObject *visitBlockNode(BlockNode *node) override;
    PyObject *visitPrintNode(PrintNode *node) override;
    PyObject *visitPassNode(PassNode *node) override;
    PyObject *visitBreakNode(BreakNode *node) override;
    PyObject *visitContinueNode(ContinueNode *node) override;
    PyObject *visitReturnNode(ReturnNode *node) override;
    PyObject *visitIfNode(IfNode *node) override;
    PyObject *visitWhileNode(WhileNode *node) override;
    PyObject *visitFunctionNode(FunctionNode *node) override;
    PyObject *visitCallNode(CallNode *node) override;
    PyObject *visitPropertyNode(PropertyNode *node) override;
    PyObject *visitClassNode(ClassNode *node) override;
    PyObject *visitIntNode(IntNode *node) override;
    PyObject *visitFloatNode(FloatNode *node) override;
    PyObject *visitStringNode(StringNode *node) override;
    PyObject *visitBooleanNode(BooleanNode *node) override;
    PyObject *visitNullNode(NullNode *node) override;
    PyObject *visitNameNode(NameNode *node) override;
    PyObject *visitBinaryOpNode(BinaryOpNode *node) override;
    PyObject *visitUnaryOpNode(UnaryOpNode *node) override;
    PyObject *visitAssignNode(AssignNode *node) override;

private:
    std::unique_ptr<Scope> globalScope;
    Scope *currentScope;
};
