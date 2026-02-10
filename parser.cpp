#include "parser.hpp"
#include <stdexcept>

Parser::Parser(const std::vector<Token> &tokens) : tokens(tokens) {}

ProgramNode *Parser::parse() { return parseProgram(); }

bool Parser::isAtEnd() const { return peek().type == TokenType::EndOfFile; }

Token Parser::peek() const { return tokens[current]; }

Token Parser::previous() const { return tokens[current - 1]; }

Token Parser::advance()
{
    if (!isAtEnd())
        current++;
    return previous();
}

bool Parser::match(TokenType type)
{
    if (peek().type == type)
    {
        advance();
        return true;
    }
    return false;
}

bool Parser::match(std::initializer_list<TokenType> types)
{
    for (TokenType type : types)
    {
        if (peek().type == type)
        {
            advance();
            return true;
        }
    }
    return false;
}

Token Parser::consume(TokenType type)
{
    if (peek().type == type)
        return advance();
    throw std::runtime_error("Expected token type " + std::to_string(static_cast<int>(type)));
}

ProgramNode *Parser::parseProgram()
{
    std::vector<AstNode *> statements = parseStmtList();
    return new ProgramNode(statements);
}

std::vector<AstNode *> Parser::parseStmtList()
{
    std::vector<AstNode *> statements;
    while (!isAtEnd())
    {
        AstNode *stmt = parseStmt();
        statements.push_back(stmt);

        // Consume trailing newlines after each statement
        while (match(TokenType::Newline))
        {
        }
    }
    return statements;
}

AstNode *Parser::parseSimpleStmt()
{
    if (match(TokenType::Print))
        return parsePrintStmt();
    if (match(TokenType::Pass))
        return new PassNode();
    if (match(TokenType::Break))
        return new BreakNode();
    if (match(TokenType::Continue))
        return new ContinueNode();
    if (match(TokenType::Return))
    {
        AstNode *value = nullptr;
        if (peek().type != TokenType::Newline && !isAtEnd())
        {
            value = parseExpr();
        }
        return new ReturnNode(value);
    }
    return parseExpr();
}

AstNode *Parser::parsePrintStmt()
{
    AstNode *expr = parseExpr();
    return new PrintNode(expr);
}

AstNode *Parser::parseClassDef()
{
    Token nameToken = consume(TokenType::Name);
    consume(TokenType::Colon);
    return new ClassNode(nameToken.lexeme, parseSuite());
}

AstNode *Parser::parseExpr()
{
    return parseAssign();
}

AstNode *Parser::parseAssign()
{
    AstNode *expr = parseOr();
    if (match(TokenType::Assign))
    {
        AstNode *value = parseAssign();
        if (expr->type == AstNodeType::Name)
        {
            NameNode *nameNode = static_cast<NameNode *>(expr);
            return new AssignNode(nameNode->name, value);
        }
        else if (expr->type == AstNodeType::Property)
        {
            PropertyNode *propNode = static_cast<PropertyNode *>(expr);
            return new PropertyAssignNode(propNode->object, propNode->property, value);
        }
        else
        {
            throw std::runtime_error("Invalid assignment target");
        }
    }
    return expr;
}

AstNode *Parser::parseOr()
{
    AstNode *left = parseAnd();
    while (match(TokenType::Or))
    {
        Token op = previous();
        left = new BinaryOpNode(left, op, parseAnd());
    }
    return left;
}

AstNode *Parser::parseAnd()
{
    AstNode *left = parseComparison();
    while (match(TokenType::And))
    {
        Token op = previous();
        left = new BinaryOpNode(left, op, parseComparison());
    }
    return left;
}

AstNode *Parser::parseComparison()
{
    AstNode *left = parseTerm();
    while (match({TokenType::EqualEqual, TokenType::BangEqual,
                  TokenType::Less, TokenType::LessEqual,
                  TokenType::Greater, TokenType::GreaterEqual}))
    {
        Token op = previous();
        left = new BinaryOpNode(left, op, parseTerm());
    }
    return left;
}

AstNode *Parser::parseTerm()
{
    AstNode *left = parseFactor();

    while (match({TokenType::Plus, TokenType::Minus}))
    {
        Token op = previous();
        AstNode *right = parseFactor();
        left = new BinaryOpNode(left, op, right);
    }

    return left;
}

AstNode *Parser::parseFactor()
{
    AstNode *left = parsePower();

    while (match({TokenType::Star, TokenType::Slash, TokenType::DoubleSlash, TokenType::Mod}))
    {
        Token op = previous();
        AstNode *right = parsePower();
        left = new BinaryOpNode(left, op, right);
    }

    return left;
}

AstNode *Parser::parsePower()
{
    AstNode *left = parseUnary();
    if (match(TokenType::DoubleStar))
    {
        Token op = previous();
        return new BinaryOpNode(left, op, parsePower());
    }
    return left;
}

AstNode *Parser::parseUnary()
{
    if (match({TokenType::Minus, TokenType::Not}))
    {
        Token op = previous();
        AstNode *operand = parseUnary();
        return new UnaryOpNode(op, operand);
    }

    return parsePrimary();
}

AstNode *Parser::parsePrimary()
{
    if (match(TokenType::Int))
        return parseCall(new IntNode(previous()));
    if (match(TokenType::Float))
        return parseCall(new FloatNode(previous()));
    if (match(TokenType::String))
        return parseCall(new StringNode(previous()));
    if (match(TokenType::True))
        return parseCall(new BooleanNode(previous()));
    if (match(TokenType::False))
        return parseCall(new BooleanNode(previous()));
    if (match(TokenType::None))
        return parseCall(new NullNode());
    if (match(TokenType::Name))
        return parseCall(new NameNode(previous()));
    if (match(TokenType::LeftParen))
    {
        AstNode *expr = parseExpr();
        consume(TokenType::RightParen);
        return parseCall(expr);
    }
    throw std::runtime_error("Expected expression");
}

AstNode *Parser::parseStmt()
{
    // Skip leading newlines
    while (match(TokenType::Newline))
    {
    }

    if (isAtEnd())
        return new PassNode();

    // Check for compound statements (if, while, def, class)
    if (match(TokenType::If))
        return parseIfStmt();
    if (match(TokenType::While))
        return parseWhileStmt();
    if (match(TokenType::Def))
        return parseFunctionDef();
    if (match(TokenType::Class))
        return parseClassDef();

    // Otherwise it's a simple statement
    AstNode *stmt = parseSimpleStmt();
    // Don't consume newline here - let parseStmtList handle it
    return stmt;
}

AstNode *Parser::parseSuite()
{
    consume(TokenType::Newline);
    consume(TokenType::Indent);

    std::vector<AstNode *> statements;
    while (!match(TokenType::Dedent) && !isAtEnd())
    {
        while (match(TokenType::Newline))
        {
        }
        if (peek().type == TokenType::Dedent)
            break;
        statements.push_back(parseStmt());
        match(TokenType::Newline);
    }
    return new BlockNode(statements);
}

AstNode *Parser::parseIfStmt()
{
    AstNode *condition = parseExpr();
    consume(TokenType::Colon);
    AstNode *thenBranch = parseSuite();

    std::vector<std::pair<AstNode *, AstNode *>> elifBranches;
    while (match(TokenType::Elif))
    {
        AstNode *elifCond = parseExpr();
        consume(TokenType::Colon);
        elifBranches.push_back({elifCond, parseSuite()});
    }

    AstNode *elseBranch = nullptr;
    if (match(TokenType::Else))
    {
        consume(TokenType::Colon);
        elseBranch = parseSuite();
    }

    return new IfNode(condition, thenBranch, elifBranches, elseBranch);
}

AstNode *Parser::parseWhileStmt()
{
    AstNode *condition = parseExpr();
    consume(TokenType::Colon);
    AstNode *body = parseSuite();
    return new WhileNode(condition, body);
}

AstNode *Parser::parseFunctionDef()
{
    Token nameToken = consume(TokenType::Name);
    consume(TokenType::LeftParen);

    std::vector<std::string> params;
    if (peek().type != TokenType::RightParen)
    {
        params.push_back(consume(TokenType::Name).lexeme);
        while (match(TokenType::Comma))
        {
            params.push_back(consume(TokenType::Name).lexeme);
        }
    }

    consume(TokenType::RightParen);
    consume(TokenType::Colon);

    return new FunctionNode(nameToken.lexeme, params, parseSuite());
}

AstNode *Parser::parseCall(AstNode *callee)
{
    while (true)
    {
        if (match(TokenType::LeftParen))
        {
            std::vector<AstNode *> args;
            if (peek().type != TokenType::RightParen)
            {
                args.push_back(parseExpr());
                while (match(TokenType::Comma))
                    args.push_back(parseExpr());
            }
            consume(TokenType::RightParen);
            callee = new CallNode(callee, args);
        }
        else if (match(TokenType::Dot))
        {
            callee = new PropertyNode(callee, consume(TokenType::Name).lexeme);
        }
        else
        {
            break;
        }
    }
    return callee;
}