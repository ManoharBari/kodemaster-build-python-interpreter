#pragma once

#include <string>
#include <memory>

class PyObject
{
public:
    virtual ~PyObject() = default;
    virtual std::string toString() const = 0;
    virtual bool isTruthy() const = 0;
};

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