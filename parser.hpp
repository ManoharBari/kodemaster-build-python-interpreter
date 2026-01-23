#pragma once

#include <vector>
#include <initializer_list>
#include "token.hpp"
#include "ast.hpp"

class Parser
{
public:
    Parser(const std::vector<Token> &tokens);
    ProgramNode *parse();

private:
    ProgramNode *parseProgram();
    std::vector<AstNode *> parseStmtList();
    AstNode *parseStmt();
    AstNode *parseSimpleStmt();
    AstNode *parsePrintStmt();

    AstNode *parseExpr();
    AstNode *parseAssign();
    AstNode *parseOr();
    AstNode *parsePower();
    AstNode *parseAnd();
    AstNode *parseComparison();
    AstNode *parseTerm();
    AstNode *parseFactor();
    AstNode *parseUnary();
    AstNode *parsePrimary();

    const std::vector<Token> &tokens;
    int current = 0;

    bool isAtEnd() const;
    Token consume(TokenType type);
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token peek() const;
    Token previous() const;
    Token advance();
};