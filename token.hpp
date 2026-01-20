#pragma once

#include <string>
#include "tokentype.hpp"

class Token
{
public:
    Token(TokenType type, const std::string &lexeme, int line)
        : type(type), lexeme(lexeme), line(line) {}

    TokenType type;
    std::string lexeme;
    int line;
};