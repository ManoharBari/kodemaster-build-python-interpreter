#include "lexer.hpp"
#include <stdexcept>

static bool isDigit(char c)
{
    return c >= '0' && c <= '9';
}

static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static bool isAlphaNumeric(char c)
{
    return isAlpha(c) || isDigit(c);
}

Lexer::Lexer(const std::string &source) : source(source), current(0), start(0), line(1)
{
    keywords["True"] = TokenType::True;
    keywords["False"] = TokenType::False;
    keywords["None"] = TokenType::None;
    keywords["and"] = TokenType::And;
    keywords["or"] = TokenType::Or;
    keywords["not"] = TokenType::Not;
    keywords["if"] = TokenType::If;
    keywords["elif"] = TokenType::Elif;
    keywords["else"] = TokenType::Else;
    keywords["while"] = TokenType::While;
    keywords["break"] = TokenType::Break;
    keywords["continue"] = TokenType::Continue;
    keywords["def"] = TokenType::Def;
    keywords["return"] = TokenType::Return;
    keywords["class"] = TokenType::Class;
    keywords["pass"] = TokenType::Pass;
    keywords["print"] = TokenType::Print;

    indentLevels.push(0);
}

std::vector<Token> Lexer::scanTokens()
{
    while (!isAtEnd())
    {
        start = current;
        scanToken();
    }
    tokens.push_back(Token(TokenType::EndOfFile, "", line));
    return tokens;
}

char Lexer::advance()
{
    return source[current++];
}

bool Lexer::isAtEnd() const
{
    return current >= source.length();
}

char Lexer::peek() const
{
    return isAtEnd() ? '\0' : source[current];
}

char Lexer::peekNext() const
{
    return (current + 1 >= source.length()) ? '\0' : source[current + 1];
}

bool Lexer::match(char expected)
{
    if (isAtEnd() || source[current] != expected)
        return false;
    current++;
    return true;
}

void Lexer::addToken(TokenType type)
{
    tokens.push_back(Token(type, source.substr(start, current - start), line));
}

void Lexer::addToken(TokenType type, const std::string &lexeme)
{
    tokens.push_back(Token(type, lexeme, line));
}

void Lexer::handleNumber()
{
    while (isDigit(peek()))
        advance();

    // Look for decimal part
    if (peek() == '.' && isDigit(peekNext()))
    {
        advance(); // consume '.'
        while (isDigit(peek()))
            advance();
        addToken(TokenType::Float);
    }
    else
    {
        addToken(TokenType::Int);
    }
}

void Lexer::handleString(char quoteType)
{
    while (peek() != quoteType && !isAtEnd())
    {
        if (peek() == '\n')
            line++;
        advance();
    }

    if (isAtEnd())
    {
        throw std::runtime_error("Unterminated string at line " + std::to_string(line));
    }

    advance(); // closing quote

    // Extract content without quotes
    std::string value = source.substr(start + 1, current - start - 2);
    addToken(TokenType::String, value);
}

void Lexer::handleIdentifier()
{
    while (isAlphaNumeric(peek()))
        advance();

    std::string text = source.substr(start, current - start);

    auto it = keywords.find(text);
    if (it != keywords.end())
    {
        addToken(it->second);
    }
    else
    {
        addToken(TokenType::Name);
    }
}

void Lexer::scanToken()
{
    char c = advance();

    switch (c)
    {
    case '(':
        addToken(TokenType::LeftParen);
        break;
    case ')':
        addToken(TokenType::RightParen);
        break;
    case ',':
        addToken(TokenType::Comma);
        break;
    case '.':
        addToken(TokenType::Dot);
        break;
    case ':':
        addToken(TokenType::Colon);
        break;

    case '+':
        addToken(TokenType::Plus);
        break;
    case '-':
        addToken(TokenType::Minus);
        break;
    case '%':
        addToken(TokenType::Mod);
        break;

    case '*':
        addToken(match('*') ? TokenType::DoubleStar : TokenType::Star);
        break;
    case '/':
        addToken(match('/') ? TokenType::DoubleSlash : TokenType::Slash);
        break;

    case '=':
        addToken(match('=') ? TokenType::EqualEqual : TokenType::Equals);
        break;
    case '!':
        if (match('='))
            addToken(TokenType::BangEqual);
        break;
    case '<':
        if (match('<'))
            addToken(TokenType::LeftShift);
        else if (match('='))
            addToken(TokenType::LessEqual);
        else
            addToken(TokenType::Less);
        break;
    case '>':
        if (match('>'))
            addToken(TokenType::RightShift);
        else if (match('='))
            addToken(TokenType::GreaterEqual);
        else
            addToken(TokenType::Greater);
        break;

    case '|':
        addToken(TokenType::Pipe);
        break;
    case '&':
        addToken(TokenType::Ampersand);
        break;
    case '^':
        addToken(TokenType::Caret);
        break;
    case '~':
        addToken(TokenType::Tilde);
        break;

    case '#':
        while (peek() != '\n' && !isAtEnd())
            advance();
        break;

    case ' ':
    case '\t':
    case '\r':
        break;

    case '\n':
        addToken(TokenType::Newline);
        line++;
        break;

    case '"':
        handleString('"');
        break;
    case '\'':
        handleString('\'');
        break;

    default:
        if (isDigit(c))
        {
            handleNumber();
        }
        else if (isAlpha(c))
        {
            handleIdentifier();
        }
        break;
    }
}