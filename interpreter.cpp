#include "interpreter.hpp"
#include <iostream>
#include <cmath>
#include "pyobject.hpp"

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

PyObject *Interpreter::visitPassNode(PassNode *)
{
    return nullptr;
}

PyObject *Interpreter::visitBreakNode(BreakNode *)
{
    throw BreakException();
}

PyObject *Interpreter::visitContinueNode(ContinueNode *)
{
    throw ContinueException();
}

PyObject *Interpreter::visitReturnNode(ReturnNode *node)
{
    std::shared_ptr<PyObject> value = node->value
                                          ? std::shared_ptr<PyObject>(node->value->accept(this))
                                          : std::make_shared<PyNone>();
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
    std::shared_ptr<AstNode> bodyPtr(node->body, [](AstNode *) {});
    std::shared_ptr<Scope> closurePtr(currentScope, [](Scope *) {});

    PyFunction *func = new PyFunction(node->name, node->params, bodyPtr, closurePtr);
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
        Scope *newCallScope = new Scope(func->closure.get());
        currentScope = newCallScope;

        size_t paramCount = func->params.size();
        for (size_t i = 0; i < paramCount; ++i)
        {
            PyObject *value = (i < args.size()) ? args[i] : new PyNone();
            currentScope->define(func->params[i], value);
        }

        PyObject *result = new PyNone();
        try
        {
            func->body->accept(this);
        }
        catch (const ReturnException &ex)
        {
            // Handle different return types
            if (auto intVal = dynamic_cast<PyInt *>(ex.value.get()))
                result = new PyInt(intVal->value);
            else if (auto floatVal = dynamic_cast<PyFloat *>(ex.value.get()))
                result = new PyFloat(floatVal->value);
            else if (auto strVal = dynamic_cast<PyStr *>(ex.value.get()))
                result = new PyStr(strVal->value);
            else if (auto boolVal = dynamic_cast<PyBool *>(ex.value.get()))
                result = new PyBool(boolVal->value);
            else if (dynamic_cast<PyNone *>(ex.value.get()))
                result = new PyNone();
            else if (auto inst = dynamic_cast<PyInstance *>(ex.value.get()))
                result = inst; // Return the instance directly
            else
                result = ex.value.get();
        }

        currentScope = previous;
        delete newCallScope;
        return result;
    }

    if (auto klass = dynamic_cast<PyClass *>(callee))
    {
        // Create shared_ptr to the class (non-owning)
        std::shared_ptr<PyClass> klassPtr(klass, [](PyClass *) {});
        PyInstance *instance = new PyInstance(klassPtr);

        std::shared_ptr<PyObject> initObj;
        try
        {
            initObj = klass->get("__init__");
        }
        catch (const std::runtime_error &)
        {
            initObj = nullptr;
        }

        if (initObj)
        {
            if (auto initFn = dynamic_cast<PyFunction *>(initObj.get()))
            {
                Scope *previous = currentScope;
                Scope *newCallScope = new Scope(initFn->closure.get());
                currentScope = newCallScope;

                size_t paramCount = initFn->params.size();
                for (size_t i = 0; i < paramCount; ++i)
                {
                    PyObject *value = nullptr;
                    if (i == 0)
                        value = instance;
                    else
                        value = (i - 1 < args.size()) ? args[i - 1] : new PyNone();
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
                delete newCallScope;
            }
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
        std::shared_ptr<PyObject> value = instance->get(node->property);

        // If it's a method (PyFunction), we need to bind self to it
        // For now, we'll just return the function and handle binding in CallNode
        // This is a simplified approach

        return value ? value.get() : static_cast<PyObject *>(new PyNone());
    }

    if (auto klass = dynamic_cast<PyClass *>(obj))
    {
        std::shared_ptr<PyObject> value = klass->get(node->property);
        return value ? value.get() : static_cast<PyObject *>(new PyNone());
    }

    return new PyNone();
}

PyObject *Interpreter::visitClassNode(ClassNode *node)
{
    Scope *previous = currentScope;
    Scope *classScope = new Scope(previous);
    currentScope = classScope;

    node->body->accept(this);

    currentScope = previous;

    PyClass *klass = new PyClass(node->name);

    // Get variables from class scope
    for (const auto &pair : classScope->getVariables())
    {
        // pair.second is std::shared_ptr<PyObject>
        if (auto func = dynamic_cast<PyFunction *>(pair.second.get()))
        {
            klass->set(pair.first, pair.second);
        }
        else
        {
            klass->set(pair.first, pair.second);
        }
    }

    currentScope->define(node->name, klass);
    delete classScope;

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

PyObject *Interpreter::visitNullNode(NullNode *)
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

    auto getNumeric = [](PyObject *obj, double &out, bool &isInt) -> bool
    {
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

    // Handle logical operators first (short-circuit)
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

    // Check for magic methods on instances
    if (auto leftInst = dynamic_cast<PyInstance *>(left))
    {
        std::string magicMethod;

        switch (node->op.type)
        {
        case TokenType::Plus:
            magicMethod = "__add__";
            break;
        case TokenType::Minus:
            magicMethod = "__sub__";
            break;
        case TokenType::Star:
            magicMethod = "__mul__";
            break;
        case TokenType::Slash:
            magicMethod = "__truediv__";
            break;
        case TokenType::LessEqual:
            magicMethod = "__le__";
            break;
        case TokenType::Less:
            magicMethod = "__lt__";
            break;
        case TokenType::GreaterEqual:
            magicMethod = "__ge__";
            break;
        case TokenType::Greater:
            magicMethod = "__gt__";
            break;
        case TokenType::EqualEqual:
            magicMethod = "__eq__";
            break;
        case TokenType::BangEqual:
            magicMethod = "__ne__";
            break;
        default:
            break;
        }

        if (!magicMethod.empty())
        {
            try
            {
                std::shared_ptr<PyObject> method = leftInst->get(magicMethod);
                if (auto func = dynamic_cast<PyFunction *>(method.get()))
                {
                    // Call the magic method with self and other
                    Scope *previous = currentScope;
                    Scope *newCallScope = new Scope(func->closure.get());
                    currentScope = newCallScope;

                    // Bind self and other
                    if (func->params.size() >= 2)
                    {
                        currentScope->define(func->params[0], leftInst);
                        currentScope->define(func->params[1], right);
                    }

                    PyObject *result = new PyNone();
                    try
                    {
                        func->body->accept(this);
                    }
                    catch (const ReturnException &ex)
                    {
                        if (auto intVal = dynamic_cast<PyInt *>(ex.value.get()))
                            result = new PyInt(intVal->value);
                        else if (auto floatVal = dynamic_cast<PyFloat *>(ex.value.get()))
                            result = new PyFloat(floatVal->value);
                        else if (auto boolVal = dynamic_cast<PyBool *>(ex.value.get()))
                            result = new PyBool(boolVal->value);
                        else if (auto inst = dynamic_cast<PyInstance *>(ex.value.get()))
                            result = inst;
                        else
                            result = ex.value.get();
                    }

                    currentScope = previous;
                    delete newCallScope;
                    return result;
                }
            }
            catch (const std::runtime_error &)
            {
                // Method not found, fall through to default behavior
            }
        }
    }

    // Default arithmetic and comparison operations
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

PyObject *Interpreter::visitPropertyAssignNode(PropertyAssignNode *node)
{
    PyObject *obj = node->object->accept(this);
    PyObject *value = node->value->accept(this);

    if (auto instance = dynamic_cast<PyInstance *>(obj))
    {
        instance->set(node->property, std::shared_ptr<PyObject>(value));
        return value;
    }

    throw std::runtime_error("Can only assign properties on instances");
}