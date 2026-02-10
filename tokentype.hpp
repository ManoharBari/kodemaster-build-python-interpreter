#pragma once

enum class TokenType {
    // Delimiters
    LeftParen, RightParen,
    Comma, Dot, Colon,

    // Arithmetic Operators
    Plus, Minus, Star, Slash, Mod,
    DoubleStar, DoubleSlash,

    // Assignment
    Assign,

    // Comparison
    EqualEqual, BangEqual,
    Less, LessEqual,
    Greater, GreaterEqual,

    // Bitwise
    Pipe, Caret, Ampersand, Tilde,
    LeftShift, RightShift,

    // Literals & Identifiers
    Int, Float, String, Name,

    // Keywords
    True, False, None,
    And, Or, Not,
    If, Elif, Else,
    While, Break, Continue,
    Def, Return, Class, Pass,

    // Built-in (optional as keyword)
    Print,

    // Layout
    Indent, Dedent, Newline,

    // End
    EndOfFile
};
