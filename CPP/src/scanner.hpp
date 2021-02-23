#pragma once

#include <string>

enum class TokenType
{
      // Single-character tokens.
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,

    // One or two character tokens.
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,

    // Literals.
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,

    // Keywords.
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,

    TOKEN_ERROR,
    TOKEN_EOF
};

struct Token
{
    TokenType type;
    int start;
    int line;
    std::string lexeme;
};

class Scanner
{

public:
    void load_source(const std::string & source);

    Token scan_token();

private:

    inline bool is_at_end()
    {
        return (*source_)[current_] == '\0';
    } 

    inline char advance()
    {
        return (*source_)[current_++];
    }

    inline char peek()
    {
        return (*source_)[current_];
    }

    inline char peek_next()
    {
        return (*source_)[current_ + 1];
    }

    bool match(char expected);

    Token make_token(TokenType type);

    Token make_error_token(std::string && message);

    Token string();
    Token number();
    Token identifier();

    void skip_whitespace();

    inline bool is_digit(char c)
    {
        return c <= '9' && c >= '0';
    }

    inline bool is_alpha(char c)
    {
        return (c <= 'z' && c >= 'a') ||
                (c <= 'Z' && c >= 'A') ||
                c == '_';
    }

    TokenType identifier_type();
    TokenType check_keyword(int start,
                            int length,
                            char const * rest,
                            TokenType type);



private:

    std::string const * source_ = nullptr;

    int start_ = 0;
    int current_ = 0;
    int line_ = 1;

};