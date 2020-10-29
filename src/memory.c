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
            // write(1, "DLRDLRLDR\n", 10);
            ObjFunction* function = (ObjFunction*)object;
            // printf("dlr xxx => [%s]\n", function->name ? function->name->chars : "SCRIPTZER");
            // printf("dlr xx arity =>[%d]\n", function->arity);
            freeChunk(&function->chunk);
            FREE(ObjFunction, function);
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
