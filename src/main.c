#include "common.h"
#include "chunk.h"
#include "vm.h"
#include "debug.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void repl()
{
    char line[1024];

    while (true)
    {
        printf("> ");

        if (!fgets(line, 1024, stdin))
        {
            printf("\n");
            break;
        }
        interpret(line);
    }
}


static char* readFile(const char* path)
{
    FILE* file = fopen(path, "rb");

    if (file == NULL)
    {
        fprintf(stderr, "Could not open file \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t fileSize = ftell(file);
    rewind(file);

    char *source = (char*)malloc(fileSize);
    if (source == NULL)
    {
        fprintf(stderr, "Not enough memory to read \"%s\".\n", path);
        exit(74);
    }
    size_t bytesRead = fread(source, sizeof(char), fileSize, file);
    if (bytesRead < fileSize)
    {
        fprintf(stderr, "Could not read file \"%s\".\n", path);
        exit(74);
    }
    source[bytesRead] = 0;
    
    if (fclose(file) == EOF)
    {
        fprintf(stderr, "Could not close file \"%s\".\n", path);
        exit(74);
    }
    return source;
}

static void runFile(char const * path)
{
    char* source = readFile(path);
    InterpretResult result = interpret(source);
    free(source);

    if (result == INTERPRET_COMPILE_ERROR) exit(65);
    if (result == INTERPRET_RUNTIME_ERROR) exit(70);
}
int main(int argc, char const * argv[])
{
    initVM();

    if (argc == 1) {
        repl();
    }
    else if (argc == 2) {
        runFile(argv[1]);
    }
    else {
        fprintf(stderr, "Usage: ./clox [path]\n");
        exit(64);
    }

    freeVM();

    return 0;

}
