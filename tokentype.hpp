#pragma once

enum class TokenType {
    // Delimiters
    LeftParen, RightParen,
    Comma, Dot, Colon,
    
    // Operators
    Plus, Minus, Star, Slash, Mod,
    DoubleStar, DoubleSlash,
    
    // Comparison
    Equals, EqualEqual, BangEqual,
    Less, LessEqual, Greater, GreaterEqual,
    
    // Bitwise
    Pipe, Caret, Ampersand, Tilde,
    LeftShift, RightShift,
    
    // Literals
    Int, Float, String, Name,
    
    // Keywords
    True, False, None,
    And, Or, Not,
    If, Elif, Else,
    While, Break, Continue,
    Def, Return, Class, Pass,
    Print,
    
    // Whitespace
    Indent, Dedent, Newline,
    
    // End of file
    EndOfFile
};