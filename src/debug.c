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
}


int disassembleInstruction(Chunk* chunk, int offset)
{
    printf("%04d ", offset);

    Line line = getLine(chunk->lines, offset); 

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
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}


