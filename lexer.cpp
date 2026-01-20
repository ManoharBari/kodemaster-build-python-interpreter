#include "lexer.hpp"
#include <stdexcept>

Lexer::Lexer(const std::string& source) : source(source) {
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

std::vector<Token> Lexer::scanTokens() {
    while (!isAtEnd()) {
        start = current;
        scanToken();
    }
    tokens.push_back(Token(TokenType::EndOfFile, "", line));
    return tokens;
}

void Lexer::scanToken() {
    char c = advance();
    // Implementation in next step
}

char Lexer::advance() { return source[current++]; }
bool Lexer::isAtEnd() const { return current >= source.length(); }
char Lexer::peek() const { return isAtEnd() ? '\0' : source[current]; }
char Lexer::peekNext() const { return (current + 1 >= source.length()) ? '\0' : source[current + 1]; }

bool Lexer::match(char expected) {
    if (isAtEnd() || source[current] != expected) return false;
    current++;
    return true;
}

void Lexer::addToken(TokenType type) {
    tokens.push_back(Token(type, source.substr(start, current - start), line));
}

void Lexer::addToken(TokenType type, const std::string& lexeme) {
    tokens.push_back(Token(type, lexeme, line));
}