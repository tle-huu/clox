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
	static int i = 0;
	switch (object->type)
	{
		case OBJ_STRING: {
			ObjString* string = (ObjString*)object;
			FREE(ObjString, object);
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
