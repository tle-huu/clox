#include "memory.h"
#include "vm.h"

#include <stdlib.h>


void* reallocate(void* pointer, int oldSize, int newSize)
{
    if (newSize == 0)
    {
        free(pointer);
        return NULL;
    }
    
    void *result = realloc(pointer, newSize);

    if (result == NULL) exit(1);

    return result;
}

static void freeObject(Obj* object)
{
    switch (object->type)
    {
        case OBJ_STRING: {
            ObjString* string = (ObjString*)object;
            FREE(ObjString, string);
            break;
        }
        case OBJ_FUNCTION: {
            ObjFunction* function = (ObjFunction*)object;
            freeChunk(&function->chunk);
            FREE(ObjFunction, function);
            break;
        }
        case OBJ_NATIVE: {
            FREE(ObjNative, object);
            break;
        }

    }
}

void freeObjects()
{
    Obj* object = vm.objects;

    while (object)
    {
        Obj* next = object->next;
        freeObject(object);
        object = next;
    }
}
