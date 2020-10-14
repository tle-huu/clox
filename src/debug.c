#include "debug.h"

#include <stdio.h>

static int simpleInstruction(const char *name, int offset)
{
    printf("%s\n", name);
    return offset + 1;
}

static int constantInstruction(const char *name, Chunk const * chunk, int offset)
{
    uint8_t constantOffset = chunk->code[offset + 1];
    printf("%-16s %4d '", name, constantOffset);
    printValue(chunk->constants.values[constantOffset]);
    printf("'\n");
    return offset + 2;
}

void disassembleChunk(Chunk *chunk, char const * name)
{
    printf("== %s ==\n", name);

    for (int offset = 0; offset < chunk->count;)
    {
        offset = disassembleInstruction(chunk, offset);
    }
    printf("== end %s ==\n", name);
}


int disassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    Line line = getLine(&chunk->lines, offset);

    if (offset > 0 && line.endingByteOffset > offset)
    {
        printf("   | ");
    }
    else
    {
        printf("%4d ", line.line);
    }

    uint8_t instruction = chunk->code[offset];

    switch (instruction)
    {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", chunk, offset);
        case OP_NEGATE:
            return simpleInstruction("OP_NEGATE", offset);
        case OP_ADD      : return simpleInstruction("OP_ADD", offset); 
        case OP_MULTIPLY : return simpleInstruction("OP_MULTIPLY", offset); 
        case OP_DIVIDE   : return simpleInstruction("OP_DIVIDE", offset); 
        case OP_SUBSTRACT: return simpleInstruction("OP_SUBSTRACT", offset); 
        case OP_NIL      : return simpleInstruction("OP_NIL", offset);
        case OP_TRUE     : return simpleInstruction("OP_TRUE", offset);
        case OP_FALSE    : return simpleInstruction("OP_FALSE", offset);
        case OP_NOT      : return simpleInstruction("OP_NOT", offset);
        case OP_EQUAL    : return simpleInstruction("OP_EQUAL", offset);
        case OP_GREATER  : return simpleInstruction("OP_GREATER", offset);
        case OP_LESS     : return simpleInstruction("OP_LESS", offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}


