#pragma once

#include <vector>
#include <string>
#include <stack>
#include <unordered_map>
#include "token.hpp"

class Lexer
{
public:
    Lexer(const std::string &source);
    std::vector<Token> scanTokens();

private:
    void scanToken();
    void handleNumber();
    char advance();
    void addToken(TokenType type);
    void addToken(TokenType type, const std::string &lexeme);
    bool match(char expected);
    char peek() const;
    char peekNext() const;
    bool isAtEnd() const;

    const std::string &source;
    std::vector<Token> tokens;
    std::unordered_map<std::string, TokenType> keywords;

    size_t start = 0;
    size_t current = 0;
    int line = 1;
    std::stack<int> indentLevels;
};