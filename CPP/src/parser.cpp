#include "parser.hpp"

#include <iostream>

void Parser::init(std::string const & source)
{
    had_error = false;
    panic_mode = false;
    scanner_.load_source(source);
}

void Parser::advance()
{
    previous = current;
    while (true)
    {
        current = scanner_.scan_token();
        if (current.type != TokenType::TOKEN_ERROR) break;

        error(current, current.lexeme.c_str());
    }
}

void Parser::consume(TokenType type, char const * message)
{
    if (current.type == type)
    {
        advance();
        return;
    }

    error_at_current(message);
}

void Parser::error(Token const & token, char const * message)
{
    if (panic_mode) return;
    panic_mode = true;
    std::cerr << "[" << token.line << "]: ";

    if (token.type == TokenType::TOKEN_EOF)
    {
        std::cerr << "at end.";
    }
    else if (token.type == TokenType::TOKEN_ERROR)
    {
        // pass
    }
    else
    {
        std::cerr << "at : " << token.lexeme << ": ";
    }
    std::cerr << message << '\n';

    had_error = true;
}

void Parser::error_at_current(char const * message)
{
    error(current, message);
}