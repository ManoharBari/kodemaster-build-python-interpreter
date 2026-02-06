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
    return nullptr;
}

PyObject *Interpreter::visitBreakNode(BreakNode *node)
{
    throw BreakException();
}

PyObject *Interpreter::visitContinueNode(ContinueNode *node)
{
    throw ContinueException();
}

PyObject *Interpreter::visitReturnNode(ReturnNode *node)
{
    PyObject *value = node->value ? node->value->accept(this) : static_cast<PyObject *>(new PyNone());
    throw ReturnException(value);
}

PyObject *Interpreter::visitIfNode(IfNode *node)
{
    if (node->condition->accept(this)->isTruthy())
    {
        node->thenBranch->accept(this);
        return new PyNone();
    }
    for (auto &elifPair : node->elifBranches)
    {
        if (elifPair.first->accept(this)->isTruthy())
        {
            elifPair.second->accept(this);
            return new PyNone();
        }
    }
    if (node->elseBranch)
        node->elseBranch->accept(this);
    return new PyNone();
}

PyObject *Interpreter::visitWhileNode(WhileNode *node)
{
    while (node->condition->accept(this)->isTruthy())
    {
        try
        {
            node->body->accept(this);
        }
        catch (const BreakException &)
        {
            break;
        }
        catch (const ContinueException &)
        {
            continue;
        }
    }
    return new PyNone();
}

PyObject *Interpreter::visitFunctionNode(FunctionNode *node)
{
    PyFunction *func = new PyFunction(node->name, node->params, node->body, currentScope);
    currentScope->define(node->name, func);
    return func;
}

PyObject *Interpreter::visitCallNode(CallNode *node)
{
    PyObject *callee = node->callee->accept(this);
    std::vector<PyObject *> args;
    args.reserve(node->args.size());
    for (AstNode *arg : node->args)
        args.push_back(arg->accept(this));

    if (auto func = dynamic_cast<PyFunction *>(callee))
    {
        Scope *previous = currentScope;
        std::unique_ptr<Scope> callScope = std::make_unique<Scope>(func->closure);
        currentScope = callScope.get();

        size_t paramCount = func->params.size();
        for (size_t i = 0; i < paramCount; ++i)
        {
            PyObject *value = (i < args.size()) ? args[i] : static_cast<PyObject *>(new PyNone());
            currentScope->define(func->params[i], value);
        }

        try
        {
            func->body->accept(this);
        }
        catch (const ReturnException &ex)
        {
            currentScope = previous;
            return ex.value;
        }
        currentScope = previous;
        return new PyNone();
    }

    if (auto klass = dynamic_cast<PyClass *>(callee))
    {
        PyInstance *instance = new PyInstance(klass);
        PyObject *initObj = klass->get("init");
        if (auto initFn = dynamic_cast<PyFunction *>(initObj))
        {
            Scope *previous = currentScope;
            std::unique_ptr<Scope> callScope = std::make_unique<Scope>(initFn->closure);
            currentScope = callScope.get();

            size_t paramCount = initFn->params.size();
            for (size_t i = 0; i < paramCount; ++i)
            {
                PyObject *value = nullptr;
                if (i == 0)
                    value = instance;
                else
                    value = (i - 1 < args.size()) ? args[i - 1] : static_cast<PyObject *>(new PyNone());
                currentScope->define(initFn->params[i], value);
            }

            try
            {
                initFn->body->accept(this);
            }
            catch (const ReturnException &)
            {
            }
            currentScope = previous;
        }
        return instance;
    }

    return new PyNone();
}

PyObject *Interpreter::visitPropertyNode(PropertyNode *node)
{
    PyObject *obj = node->object->accept(this);
    if (auto instance = dynamic_cast<PyInstance *>(obj))
    {
        PyObject *value = instance->get(node->property);
        return value ? value : static_cast<PyObject *>(new PyNone());
    }
    if (auto klass = dynamic_cast<PyClass *>(obj))
    {
        PyObject *value = klass->get(node->property);
        return value ? value : static_cast<PyObject *>(new PyNone());
    }
    return new PyNone();
}

PyObject *Interpreter::visitClassNode(ClassNode *node)
{
    Scope *previous = currentScope;
    std::unique_ptr<Scope> classScope = std::make_unique<Scope>(previous);
    currentScope = classScope.get();
    node->body->accept(this);
    currentScope = previous;

    PyClass *klass = new PyClass(node->name);
    for (const auto &pair : classScope->getVariables())
    {
        klass->set(pair.first, pair.second);
    }
    currentScope->define(node->name, klass);
    return klass;
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

    auto getNumeric = [](PyObject *obj, double &out, bool &isInt) -> bool {
        if (auto v = dynamic_cast<PyInt *>(obj))
        {
            out = static_cast<double>(v->value);
            isInt = true;
            return true;
        }
        if (auto v = dynamic_cast<PyFloat *>(obj))
        {
            out = v->value;
            isInt = false;
            return true;
        }
        if (auto v = dynamic_cast<PyBool *>(obj))
        {
            out = v->value ? 1.0 : 0.0;
            isInt = true;
            return true;
        }
        return false;
    };

    switch (node->op.type)
    {
    case TokenType::And:
    {
        if (!left->isTruthy())
            return new PyBool(false);
        PyObject *right = node->right->accept(this);
        return new PyBool(right->isTruthy());
    }
    case TokenType::Or:
    {
        if (left->isTruthy())
            return new PyBool(true);
        PyObject *right = node->right->accept(this);
        return new PyBool(right->isTruthy());
    }
    default:
        break;
    }

    PyObject *right = node->right->accept(this);

    if (node->op.type == TokenType::Plus)
    {
        if (auto l = dynamic_cast<PyStr *>(left))
        {
            if (auto r = dynamic_cast<PyStr *>(right))
                return new PyStr(l->value + r->value);
            return new PyNone();
        }

        double lv, rv;
        bool li, ri;
        if (getNumeric(left, lv, li) && getNumeric(right, rv, ri))
        {
            if (li && ri)
                return new PyInt(static_cast<long long>(lv + rv));
            return new PyFloat(lv + rv);
        }
        return new PyNone();
    }

    if (node->op.type == TokenType::Minus || node->op.type == TokenType::Star ||
        node->op.type == TokenType::Slash || node->op.type == TokenType::DoubleSlash ||
        node->op.type == TokenType::Mod || node->op.type == TokenType::DoubleStar)
    {
        if (node->op.type == TokenType::Star)
        {
            if (auto l = dynamic_cast<PyStr *>(left))
            {
                if (auto r = dynamic_cast<PyInt *>(right))
                {
                    if (r->value <= 0)
                        return new PyStr("");
                    std::string out;
                    out.reserve(l->value.size() * static_cast<size_t>(r->value));
                    for (long long i = 0; i < r->value; ++i)
                        out += l->value;
                    return new PyStr(out);
                }
            }
            if (auto r = dynamic_cast<PyStr *>(right))
            {
                if (auto l = dynamic_cast<PyInt *>(left))
                {
                    if (l->value <= 0)
                        return new PyStr("");
                    std::string out;
                    out.reserve(r->value.size() * static_cast<size_t>(l->value));
                    for (long long i = 0; i < l->value; ++i)
                        out += r->value;
                    return new PyStr(out);
                }
            }
        }

        double lv, rv;
        bool li, ri;
        if (!getNumeric(left, lv, li) || !getNumeric(right, rv, ri))
            return new PyNone();

        switch (node->op.type)
        {
        case TokenType::Minus:
            if (li && ri)
                return new PyInt(static_cast<long long>(lv - rv));
            return new PyFloat(lv - rv);
        case TokenType::Star:
            if (li && ri)
                return new PyInt(static_cast<long long>(lv * rv));
            return new PyFloat(lv * rv);
        case TokenType::Slash:
            return new PyFloat(lv / rv);
        case TokenType::DoubleSlash:
        {
            double q = std::floor(lv / rv);
            if (li && ri)
                return new PyInt(static_cast<long long>(q));
            return new PyFloat(q);
        }
        case TokenType::Mod:
        {
            double q = std::floor(lv / rv);
            double res = lv - q * rv;
            if (li && ri)
                return new PyInt(static_cast<long long>(res));
            return new PyFloat(res);
        }
        case TokenType::DoubleStar:
        {
            double res = std::pow(lv, rv);
            if (li && ri)
                return new PyInt(static_cast<long long>(res));
            return new PyFloat(res);
        }
        default:
            break;
        }
    }

    if (node->op.type == TokenType::EqualEqual || node->op.type == TokenType::BangEqual ||
        node->op.type == TokenType::Less || node->op.type == TokenType::LessEqual ||
        node->op.type == TokenType::Greater || node->op.type == TokenType::GreaterEqual)
    {
        bool result = false;
        if (auto l = dynamic_cast<PyStr *>(left))
        {
            if (auto r = dynamic_cast<PyStr *>(right))
            {
                if (node->op.type == TokenType::EqualEqual)
                    result = (l->value == r->value);
                else if (node->op.type == TokenType::BangEqual)
                    result = (l->value != r->value);
                else if (node->op.type == TokenType::Less)
                    result = (l->value < r->value);
                else if (node->op.type == TokenType::LessEqual)
                    result = (l->value <= r->value);
                else if (node->op.type == TokenType::Greater)
                    result = (l->value > r->value);
                else if (node->op.type == TokenType::GreaterEqual)
                    result = (l->value >= r->value);
                return new PyBool(result);
            }
        }

        double lv, rv;
        bool li, ri;
        if (getNumeric(left, lv, li) && getNumeric(right, rv, ri))
        {
            if (node->op.type == TokenType::EqualEqual)
                result = (lv == rv);
            else if (node->op.type == TokenType::BangEqual)
                result = (lv != rv);
            else if (node->op.type == TokenType::Less)
                result = (lv < rv);
            else if (node->op.type == TokenType::LessEqual)
                result = (lv <= rv);
            else if (node->op.type == TokenType::Greater)
                result = (lv > rv);
            else if (node->op.type == TokenType::GreaterEqual)
                result = (lv >= rv);
            return new PyBool(result);
        }

        if (dynamic_cast<PyNone *>(left) && dynamic_cast<PyNone *>(right))
        {
            if (node->op.type == TokenType::EqualEqual)
                return new PyBool(true);
            if (node->op.type == TokenType::BangEqual)
                return new PyBool(false);
        }

        if (node->op.type == TokenType::EqualEqual)
            return new PyBool(false);
        if (node->op.type == TokenType::BangEqual)
            return new PyBool(true);
        return new PyBool(false);
    }

    return new PyNone();
}

PyObject *Interpreter::visitUnaryOpNode(UnaryOpNode *node)
{
    PyObject *operand = node->operand->accept(this);

    if (node->op.type == TokenType::Not)
        return new PyBool(!operand->isTruthy());

    if (node->op.type == TokenType::Minus)
    {
        if (auto v = dynamic_cast<PyInt *>(operand))
            return new PyInt(-v->value);
        if (auto v = dynamic_cast<PyFloat *>(operand))
            return new PyFloat(-v->value);
        if (auto v = dynamic_cast<PyBool *>(operand))
            return new PyInt(v->value ? -1 : 0);
    }
    return new PyNone();
}

PyObject *Interpreter::visitAssignNode(AssignNode *node)
{
    PyObject *value = node->value->accept(this);
    currentScope->set(node->name.lexeme, value);
    return value;
}
