#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include "pyobject.hpp"

class Scope
{
public:
    Scope(Scope *enclosing = nullptr) : enclosing(enclosing) {}

    void define(const std::string &name, PyObject *value)
    {
        // Create non-owning shared_ptr to avoid double-delete
        // The caller is responsible for memory management
        variables[name] = std::shared_ptr<PyObject>(value, [](PyObject *) {
            // Empty deleter - don't actually delete
        });
    }

    PyObject *get(const std::string &name)
    {
        if (variables.find(name) != variables.end())
        {
            return variables[name].get();
        }
        if (enclosing != nullptr)
        {
            return enclosing->get(name);
        }
        throw std::runtime_error("Undefined variable '" + name + "'");
    }

    void set(const std::string &name, PyObject *value)
    {
        if (variables.find(name) != variables.end())
        {
            variables[name] = std::shared_ptr<PyObject>(value, [](PyObject *) {});
            return;
        }
        if (enclosing != nullptr)
        {
            enclosing->set(name, value);
            return;
        }
        // If not found anywhere, create in current scope
        define(name, value);
    }

    const std::unordered_map<std::string, std::shared_ptr<PyObject>> &getVariables() const
    {
        return variables;
    }

private:
    Scope *enclosing;
    std::unordered_map<std::string, std::shared_ptr<PyObject>> variables;
};