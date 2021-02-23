#include "scanner.hpp"

#include <cstring>
#include <iostream>
#include <string>

void Scanner::load_source(std::string const & source)
{
    source_ = &source;
    start_ = 0;
    current_ = 0;
    line_ = 1;
}

Token Scanner::scan_token()
{
    if (source_ == nullptr)
    {
        std::cerr << "Scanner: Error no source has been loaded.\n";
        return make_token(TokenType::TOKEN_ERROR);
    }

    skip_whitespace();

    if (is_at_end()) return make_token(TokenType::TOKEN_EOF);

    start_ = current_;

    char c = advance();
    std::cout << "xxx [" << c << "]\n";


    if (is_digit(c)) return number();
    if (is_alpha(c)) return identifier();
    switch (c)
    {
        case '(': return make_token(TokenType::TOKEN_LEFT_PAREN);
        case ')': return make_token(TokenType::TOKEN_RIGHT_PAREN);
        case '{': return make_token(TokenType::TOKEN_LEFT_BRACE);
        case '}': return make_token(TokenType::TOKEN_RIGHT_BRACE);
        case ';': return make_token(TokenType::TOKEN_SEMICOLON);
        case ',': return make_token(TokenType::TOKEN_COMMA);
        case '.': return make_token(TokenType::TOKEN_DOT);
        case '-': return make_token(TokenType::TOKEN_MINUS);
        case '+': return make_token(TokenType::TOKEN_PLUS);
        case '/': return make_token(TokenType::TOKEN_SLASH);
        case '*': return make_token(TokenType::TOKEN_STAR);
        case '!':
            return make_token(match('=') ? TokenType::TOKEN_BANG_EQUAL : TokenType::TOKEN_BANG);
        case '=':
            return make_token(match('=') ? TokenType::TOKEN_EQUAL_EQUAL : TokenType::TOKEN_EQUAL);
        case '<':
            return make_token(match('=') ? TokenType::TOKEN_LESS_EQUAL : TokenType::TOKEN_LESS);
        case '>':
            return make_token(match('=') ? TokenType::TOKEN_GREATER_EQUAL : TokenType::TOKEN_GREATER);

        case '"': return string();

        default:
            make_error_token("Unexpected token");
    }

    return make_error_token("Unexpected character");
}

Token Scanner::make_token(TokenType type)
{
    Token token;

    token.type = type;
    token.start = start_;
    token.line = line_;
    token.lexeme = source_->substr(start_, current_ - start_) ;
    return token;
}

Token Scanner::make_error_token(std::string && message)
{
    Token token;

    token.type = TokenType::TOKEN_ERROR;
    token.start = start_;
    token.line = line_;
    token.lexeme = message;
    return token;
}

bool Scanner::match(char expected)
{
    if (is_at_end()) return false;
    if (peek() != expected) return false;

    ++current_;
    return true;
}

void Scanner::skip_whitespace()
{
    while (true)
    {
        char c = peek();
        switch (c)
        {
            case ' ':
            case '\t':
            case '\r':
                advance();
                break;
            case '/':
                if (peek_next() == '/')
                {
                    while (peek() != '\n' && !is_at_end()) advance();
                }
                else
                {
                    return;
                }
                break;
            case '\n':
                ++line_;
                advance();
                break;

            default:
                return;
        }
    }
}

Token Scanner::string()
{
    while (peek() != '"' && !is_at_end())
    {
        if (peek() == '\n') ++line_;
        
        advance();
    }

    if (is_at_end())
    {
        return make_error_token("Unterminated string.");
    }
    advance();

    return make_token(TokenType::TOKEN_STRING);
}

Token Scanner::number()
{
    while (is_digit(peek())) advance();

    if (peek() == '.' && is_digit(peek_next()))
    {
        advance();
        while (is_digit(peek())) advance();
    }

    return make_token(TokenType::TOKEN_NUMBER);
}

Token Scanner::identifier()
{
    while (is_alpha(peek()) || is_digit(peek())) advance();

    return make_token(identifier_type());
}

TokenType Scanner::identifier_type()
{
    switch ((*source_)[start_])
    {
        case 'a': return check_keyword(1, 2, "nd", TokenType::TOKEN_AND);
        case 'c': return check_keyword(1, 4, "lass", TokenType::TOKEN_CLASS);
        case 'e': return check_keyword(1, 3, "lse", TokenType::TOKEN_ELSE);
        case 'i': return check_keyword(1, 1, "f", TokenType::TOKEN_IF);
        case 'n': return check_keyword(1, 2, "il", TokenType::TOKEN_NIL);
        case 'o': return check_keyword(1, 1, "r", TokenType::TOKEN_OR);
        case 'p': return check_keyword(1, 4, "rint", TokenType::TOKEN_PRINT);
        case 'r': return check_keyword(1, 5, "eturn", TokenType::TOKEN_RETURN);
        case 's': return check_keyword(1, 4, "uper", TokenType::TOKEN_SUPER);
        case 'v': return check_keyword(1, 2, "ar", TokenType::TOKEN_VAR);
        case 'w': return check_keyword(1, 4, "hile", TokenType::TOKEN_WHILE);

        case 'f':
            if (current_ - start_ > 1)
            {
                switch ((*source_)[start_ + 1])
                {
                    case 'a': return check_keyword(2, 3, "lse",TokenType::TOKEN_FALSE);
                    case 'o': return check_keyword(2, 1, "r",TokenType::TOKEN_FOR);
                    case 'u': return check_keyword(2, 1, "n",TokenType::TOKEN_FUN);
                }
            }
            break;
        case 't':
            if (current_ - start_ > 1)
            {
                switch ((*source_)[start_ + 1])
                {
                    case 'h': return check_keyword(2, 2, "is",TokenType::TOKEN_THIS);
                    case 'r': return check_keyword(2, 2, "ue",TokenType::TOKEN_TRUE);
                }
            }
            break;
    }
    return TokenType::TOKEN_IDENTIFIER;
}

TokenType Scanner::check_keyword(int start, int length, char const * rest, TokenType type)
{
    if (current_ - start_ == start + length && memcmp(rest, (*source_).c_str() + start_ + start, length) == 0)
    {
        return type;
    }
    return TokenType::TOKEN_IDENTIFIER;
}