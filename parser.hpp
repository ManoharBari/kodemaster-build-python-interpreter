#pragma once

#include "ast.hpp"
#include "token.hpp"
#include <vector>
#include <initializer_list>

class Parser
{
public:
    Parser(const std::vector<Token> &tokens);
    ProgramNode *parse();

private:
    const std::vector<Token> &tokens;
    size_t current = 0;

    bool isAtEnd() const;
    Token peek() const;
    Token previous() const;
    Token advance();
    bool match(TokenType type);
    bool match(std::initializer_list<TokenType> types);
    Token consume(TokenType type);
    void skipNewlines(); // ← NEW METHOD

    ProgramNode *parseProgram();
    std::vector<AstNode *> parseStmtList();
    AstNode *parseStmt();
    AstNode *parseSimpleStmt();
    AstNode *parseSuite();
    AstNode *parsePrintStmt();
    AstNode *parseIfStmt();
    AstNode *parseWhileStmt();
    AstNode *parseFunctionDef();
    AstNode *parseClassDef();
    AstNode *parseExpr();
    AstNode *parseAssign();
    AstNode *parseOr();
    AstNode *parseAnd();
    AstNode *parseComparison();
    AstNode *parseTerm();
    AstNode *parseFactor();
    AstNode *parsePower();
    AstNode *parseUnary();
    AstNode *parsePrimary();
    AstNode *parseCall(AstNode *callee);
};