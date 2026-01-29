#include "interpreter.hpp"
#include <iostream>
#include <cmath>

Interpreter::Interpreter()
{
    globalScope = std::make_unique<Scope>();
    currentScope = globalScope.get();
}

void Interpreter::interpret(ProgramNode *program)
{
    program->accept(this);
}

PyObject *Interpreter::visitProgramNode(ProgramNode *node)
{
    for (AstNode *stmt : node->statements)
    {
        stmt->accept(this);
    }
    return new PyNone();
}

PyObject *Interpreter::visitBlockNode(BlockNode *node)
{
    for (AstNode *stmt : node->statements)
    {
        stmt->accept(this);
    }
    return new PyNone();
}

PyObject *Interpreter::visitPrintNode(PrintNode *node)
{
    PyObject *value = node->expression->accept(this);
    std::cout << value->toString() << std::endl;
    return new PyNone();
}

PyObject *Interpreter::visitPassNode(PassNode *node)
{
    return new PyNone();
}

PyObject *Interpreter::visitBreakNode(BreakNode *node)
{
    // TODO: Implement with exception handling
    return new PyNone();
}

PyObject *Interpreter::visitContinueNode(ContinueNode *node)
{
    // TODO: Implement with exception handling
    return new PyNone();
}

PyObject *Interpreter::visitReturnNode(ReturnNode *node)
{
    // TODO: Implement with exception handling
    if (node->value)
    {
        return node->value->accept(this);
    }
    return new PyNone();
}

PyObject *Interpreter::visitIfNode(IfNode *node)
{
    // TODO: Implement
    return new PyNone();
}

PyObject *Interpreter::visitWhileNode(WhileNode *node)
{
    // TODO: Implement
    return new PyNone();
}

PyObject *Interpreter::visitFunctionNode(FunctionNode *node)
{
    // TODO: Implement
    return new PyNone();
}

PyObject *Interpreter::visitCallNode(CallNode *node)
{
    // TODO: Implement
    return new PyNone();
}

PyObject *Interpreter::visitPropertyNode(PropertyNode *node)
{
    // TODO: Implement
    return new PyNone();
}

PyObject *Interpreter::visitClassNode(ClassNode *node)
{
    // TODO: Implement
    return new PyNone();
}

PyObject *Interpreter::visitIntNode(IntNode *node)
{
    return new PyInt(std::stoll(node->value.lexeme));
}

PyObject *Interpreter::visitFloatNode(FloatNode *node)
{
    return new PyFloat(std::stod(node->value.lexeme));
}

PyObject *Interpreter::visitStringNode(StringNode *node)
{
    return new PyStr(node->value.lexeme);
}

PyObject *Interpreter::visitBooleanNode(BooleanNode *node)
{
    return new PyBool(node->value.type == TokenType::True);
}

PyObject *Interpreter::visitNullNode(NullNode *node)
{
    return new PyNone();
}

PyObject *Interpreter::visitNameNode(NameNode *node)
{
    return currentScope->get(node->name.lexeme);
}

PyObject *Interpreter::visitBinaryOpNode(BinaryOpNode *node)
{
    PyObject *left = node->left->accept(this);
    PyObject *right = node->right->accept(this);

    // TODO: Implement binary operations
    return new PyNone();
}

PyObject *Interpreter::visitUnaryOpNode(UnaryOpNode *node)
{
    PyObject *operand = node->operand->accept(this);

    // TODO: Implement unary operations
    return new PyNone();
}

PyObject *Interpreter::visitAssignNode(AssignNode *node)
{
    PyObject *value = node->value->accept(this);
    currentScope->set(node->name.lexeme, value);
    return value;
}