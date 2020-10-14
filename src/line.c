#include "line.h"
#include "memory.h"

void initLineArray(LineArray* array)
{
    array->count = 0;
    array->capacity = 0;
    array->array = 0;
}

void freeLineArray(LineArray* array)
{
    FREE_ARRAY(Line, array->array, array->capacity);
    initLineArray(array);
}

void writeLineArray(LineArray* lineArray, int byteOffset, int line)
{
    if (lineArray->capacity < lineArray->count + 1)
    {
        int oldCapacity = lineArray->capacity;
        lineArray->capacity = GROW_CAPACITY(oldCapacity);
        lineArray->array = GROW_ARRAY(Line, lineArray->array, oldCapacity, lineArray->capacity);
    }

    if (lineArray->count > 0 && lineArray->array[lineArray->count - 1].line == line)
    {
        lineArray->array[lineArray->count - 1].endingByteOffset = byteOffset;
    }
    else
    {
       lineArray->array[lineArray->count].line = line; 
       lineArray->array[lineArray->count].endingByteOffset = byteOffset; 
       lineArray->count++;
    }

}

Line getLine(LineArray* lineArray, int byteOffset)
{
    int lineIndex = 0;

    while (lineIndex < lineArray->count && lineArray->array[lineIndex].endingByteOffset < byteOffset)
    {
        lineIndex++;
    }

   return lineArray->array[lineIndex]; 
}
