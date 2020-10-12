#ifndef clox_chunk_h

#define clox_chunk_h

#include "common.h"
#include "value.h"
#include "line.h"

typedef enum {
    OP_RETURN,
    OP_CONSTANT,
} OpCode;



typedef struct {
    int count;
    int capacity;
    uint8_t* code;
    ValueArray constants;
    LineArray *lines;
} Chunk;



void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
int addConstant(Chunk* chunk, Value value);

#endif
