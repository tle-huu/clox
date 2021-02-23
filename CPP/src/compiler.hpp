#pragma once

#include "scanner.hpp"
#include "chunk.hpp"
#include "parser.hpp"
#include "value.hpp"

#include <functional>

enum Precedence
{
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY,
};

class Compiler
{

public:
    bool compile(std::string const & source, Chunk & chunk);

private:

    void parse_precedence(Precedence precedence);

    uint8_t make_constant(Value const value);

    inline void load_chunk(Chunk & chunk) { chunk_ = &chunk; }

    inline void emit_byte(uint8_t byte)
    {
        chunk_->write(byte, parser_.current.line);
    }

    inline void emit_bytes(uint8_t byte_1, uint8_t byte_2)
    {
        emit_byte(byte_1);
        emit_byte(byte_2);
    }

    inline void emit_byte(OpCode opcode)
    {
        emit_byte(static_cast<uint8_t>(opcode));
    }

    inline void emit_constant(Value const value)
    {
        emit_bytes(static_cast<uint8_t>(OpCode::OP_CONSTANT), make_constant(value));
    }

    inline void emit_return()
    {
        emit_byte(OpCode::OP_RETURN);
    }

    inline void end_compiler()
    {
        emit_return();
    }

    void unary();
    void number();
    void grouping();

    void binary();

    void expression();

    Precedence get_current_precedence();

private:
    Parser parser_;

    Chunk * chunk_ = nullptr;

    using ParseFn = void (Compiler::*)(void);

    struct ParseRule
    {
        ParseFn prefix;
        ParseFn infix;
        Precedence precedence;
    };

    ParseRule const RULES_[40] = {
      [static_cast<uint8_t>(TokenType::TOKEN_LEFT_PAREN)]    = {&Compiler::grouping, NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_RIGHT_PAREN)]   = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_LEFT_BRACE)]    = {NULL,     NULL,   PREC_NONE}, 
      [static_cast<uint8_t>(TokenType::TOKEN_RIGHT_BRACE)]   = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_COMMA)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_DOT)]           = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_MINUS)]         = {&Compiler::unary,    &Compiler::binary, PREC_TERM},
      [static_cast<uint8_t>(TokenType::TOKEN_PLUS)]          = {NULL,     &Compiler::binary, PREC_TERM},
      [static_cast<uint8_t>(TokenType::TOKEN_SEMICOLON)]     = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_SLASH)]         = {NULL,     &Compiler::binary, PREC_FACTOR},
      [static_cast<uint8_t>(TokenType::TOKEN_STAR)]          = {NULL,     &Compiler::binary, PREC_FACTOR},
      [static_cast<uint8_t>(TokenType::TOKEN_BANG)]          = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_BANG_EQUAL)]    = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_EQUAL)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_EQUAL_EQUAL)]   = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_GREATER)]       = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_GREATER_EQUAL)] = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_LESS)]          = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_LESS_EQUAL)]    = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_IDENTIFIER)]    = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_STRING)]        = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_NUMBER)]        = {&Compiler::number,   NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_AND)]           = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_CLASS)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_ELSE)]          = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_FALSE)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_FOR)]           = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_FUN)]           = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_IF)]            = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_NIL)]           = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_OR)]            = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_PRINT)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_RETURN)]        = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_SUPER)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_THIS)]          = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_TRUE)]          = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_VAR)]           = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_WHILE)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_ERROR)]         = {NULL,     NULL,   PREC_NONE},
      [static_cast<uint8_t>(TokenType::TOKEN_EOF)]           = {NULL,     NULL,   PREC_NONE},
    };
};