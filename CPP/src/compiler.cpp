#include "chunk.hpp"
#include "compiler.hpp"

#include <iomanip> 
#include <iostream>
#include <string>

bool Compiler::compile(std::string const & source, Chunk & chunk)
{
    load_chunk(chunk);
    parser_.init(source);
    parser_.advance();

    expression();
    parser_.consume(TokenType::TOKEN_EOF, "Expected end of expression.");

    end_compiler();

    return !parser_.had_error;
}


uint8_t Compiler::make_constant(Value const value)
{
    int constant = chunk_->add_constant(value);
    if (constant > UINT8_MAX)
    {
        parser_.error_at_current("Too many constant in chunk.");
        return 0;
    }

    return static_cast<uint8_t>(constant);
}

void Compiler::expression()
{
    parse_precedence(PREC_ASSIGNMENT);
    parser_.consume(TokenType::TOKEN_EOF, "Expect end of file.");
}

void Compiler::unary()
{
    TokenType operator_type = parser_.previous.type;

    parse_precedence(PREC_UNARY);

    switch (operator_type)
    {
        case TokenType::TOKEN_MINUS:
            emit_byte(OpCode::OP_NEGATE);
            break;
        default:
            return;
    }
}

void Compiler::number()
{
    Value value = std::stod(parser_.previous.lexeme, NULL);
    emit_constant(value);
}

void Compiler::grouping()
{
    expression();
    parser_.consume(TokenType::TOKEN_RIGHT_PAREN, "Expecting closing parenthesis after expression.");
}

void Compiler::binary()
{
    TokenType operator_type = parser_.previous.type;

    parse_precedence(PREC_TERM);
    switch (operator_type)
    {
        case TokenType::TOKEN_PLUS:
            emit_byte(OpCode::OP_ADD);
            break;
        case TokenType::TOKEN_MINUS:
            emit_byte(OpCode::OP_SUBSTRACT);
            break;
        case TokenType::TOKEN_SLASH:
            emit_byte(OpCode::OP_DIVIDE);
            break;
        case TokenType::TOKEN_STAR:
            emit_byte(OpCode::OP_MULTIPLY);
            break;
        default:
            return;

    }
}

void Compiler::parse_precedence(Precedence precedence)
{
    parser_.advance();
    TokenType type = parser_.previous.type;

    ParseFn parselet = RULES_[static_cast<int>(type)].prefix;

    (this->*parselet)();

    while (precedence < get_current_precedence())
    {
        parser_.advance();
        ParseFn infix_parselet = RULES_[static_cast<int>(parser_.previous.type)].infix;

        (this->*infix_parselet)();
    }
}

Precedence Compiler::get_current_precedence()
{
    return RULES_[static_cast<int>(parser_.peek().type)].precedence;
}
