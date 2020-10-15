#ifndef clox_compiler_h
#define clox_compiler_h

#include "chunk.h"
#include "object.h"
#include "scanner.h"
#include "table.h"

typedef struct
{
    Token current;
    Token previous;
    bool hadError;
    bool panicMode;
    Table globals;
} Parser;

extern Parser parser;

bool compile(const char* source, Chunk* chunk);

#endif
