#ifndef clox_line_h

#define clox_line_h

typedef struct {
    int line;
    int endingByteOffset;
} Line;

typedef struct {
    
    int count;
    int capacity;
    Line* array;

} LineArray;


void initLineArray(LineArray* array);
void freeLineArray(LineArray* array);
void writeLineArray(LineArray* array, int byteOffset, int line);
Line getLine(LineArray* lineArray, int byteOffset);


#endif
