#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>
#include <stdexcept>

// Forward declarations
class AstNode;
class Scope;

// ==================== Control Flow Exceptions ====================
struct BreakException : public std::exception {};

struct ContinueException : public std::exception {};

struct ReturnException : public std::exception
{
    std::shared_ptr<class PyObject> value;
    ReturnException(std::shared_ptr<class PyObject> val) : value(val) {}
};

// ==================== Base PyObject ====================
class PyObject
{
public:
    virtual ~PyObject() = default;
    virtual std::string toString() const = 0;
    virtual bool isTruthy() const = 0;
};

// ==================== Basic Types ====================
class PyInt : public PyObject
{
public:
    PyInt(long long value) : value(value) {}
    std::string toString() const override { return std::to_string(value); }
    bool isTruthy() const override { return value != 0; }
    long long value;
};

class PyFloat : public PyObject
{
public:
    PyFloat(double value) : value(value) {}
    std::string toString() const override { return std::to_string(value); }
    bool isTruthy() const override { return value != 0.0; }
    double value;
};

class PyStr : public PyObject
{
public:
    PyStr(const std::string &value) : value(value) {}
    std::string toString() const override { return value; }
    bool isTruthy() const override { return !value.empty(); }
    std::string value;
};

class PyBool : public PyObject
{
public:
    PyBool(bool value) : value(value) {}
    std::string toString() const override { return value ? "True" : "False"; }
    bool isTruthy() const override { return value; }
    bool value;
};

class PyNone : public PyObject
{
public:
    std::string toString() const override { return "None"; }
    bool isTruthy() const override { return false; }
};

// ==================== PyFunction ====================
class PyFunction : public PyObject
{
public:
    std::string name;
    std::vector<std::string> params;
    std::shared_ptr<AstNode> body;
    std::shared_ptr<Scope> closure; // Lexical scope where function was defined

    PyFunction(const std::string &name,
               const std::vector<std::string> &params,
               std::shared_ptr<AstNode> body,
               std::shared_ptr<Scope> closure)
        : name(name), params(params), body(body), closure(closure) {}

    std::string toString() const override
    {
        return "<function " + name + ">";
    }

    bool isTruthy() const override { return true; }
};

// ==================== PyClass ====================
class PyClass : public PyObject
{
public:
    std::string name;
    std::map<std::string, std::shared_ptr<PyObject>> methods;

    PyClass(const std::string &name) : name(name) {}

    std::string toString() const override
    {
        return "<class '" + name + "'>";
    }

    bool isTruthy() const override { return true; }
};

// ==================== PyInstance ====================
class PyInstance : public PyObject
{
public:
    PyClass *klass;
    std::map<std::string, std::shared_ptr<PyObject>> attributes;

    PyInstance(PyClass *klass) : klass(klass) {}

    std::shared_ptr<PyObject> get(const std::string &name)
    {
        // First check instance attributes
        auto it = attributes.find(name);
        if (it != attributes.end())
        {
            return it->second;
        }

        // Then check class methods
        auto method_it = klass->methods.find(name);
        if (method_it != klass->methods.end())
        {
            return method_it->second;
        }

        throw std::runtime_error("Attribute '" + name + "' not found");
    }

    void set(const std::string &name, std::shared_ptr<PyObject> value)
    {
        attributes[name] = value;
    }

    std::string toString() const override
    {
        return "<" + klass->name + " instance>";
    }

    bool isTruthy() const override { return true; }
};