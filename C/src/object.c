#include "object.h"
#include "memory.h"
#include "table.h"
#include "vm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define ALLOCATE_OBJ_SIZE(type, rawSize, objectType) \
    (type*)allocateObject(rawSize, objectType)

#define ALLOCATE_OBJ(type, objectType) \
    ALLOCATE_OBJ_SIZE(type, sizeof(type), objectType)

static Obj* allocateObject(size_t size, ObjType type)
{
    Obj* object = (Obj*)malloc(size);
    object->type = type;

    object->next = vm.objects;
    vm.objects = object;
    return object;
}

static uint32_t hashString(const char* chars, int length)
{
    uint32_t hash = 2166136261u;

    for (int i = 0; i < length; ++i)
    {
        hash ^= chars[i];
        hash *= 16777619;
    }
    return hash;
}

static ObjString* allocateString(int length, uint32_t hash)
{
    ObjString* string = ALLOCATE_OBJ_SIZE(ObjString, sizeof(ObjString) + length + 1, OBJ_STRING);
    string->length = length;
    string->hash = hash;
    tableSet(&vm.strings, string, NIL_VAL);
    return string;
}

ObjFunction* newFunction()
{
    ObjFunction* function = ALLOCATE_OBJ(ObjFunction, OBJ_FUNCTION);

    function->arity = 0;
    function->obj.type = OBJ_FUNCTION;
    function->name = NULL;
    initChunk(&function->chunk);
    return function;
}

ObjNative* newNative(NativeFn function, int arity)
{
    ObjNative* native = ALLOCATE_OBJ(ObjNative, OBJ_NATIVE);
    native->function = function;
    native->arity = arity;
    return native;
}

ObjString* copyString(const char* chars, int length)
{
    uint32_t hash = hashString(chars, length);

    ObjString* interned = tableFindString(&vm.strings, chars, length, hash);

    if (interned != NULL) return interned;

    ObjString* string = allocateString(length, hash);
    memcpy(string->chars, chars, length + 1);
    string->chars[length] = 0;
    return string;
}

ObjString* concatenateStrings(const ObjString* a, const ObjString* b)
{
    ObjString* string = allocateString(a->length + b->length, -1);
    memcpy(string->chars, a->chars, a->length);
    memcpy(string->chars + a->length, b->chars, b->length + 1);

    int hash = hashString(string->chars, string->length);

    ObjString* interned = tableFindString(&vm.strings, string->chars, string->length, hash);

    if (interned != NULL) return interned;

    string->hash = hash;

    return string;
}

void printFunction(ObjFunction* function)
{
    if (function->name == NULL)
    {
        printf("<script>");
    }
    else
    {
        printf("<fn %s>", function->name->chars);
    }
}

void printObject(Obj* object)
{
    switch (object->type)
    {
        case OBJ_STRING:
            printf("%s", ((ObjString*)object)->chars);
            break;
        case OBJ_FUNCTION:
            printFunction((ObjFunction*)object);
            break;
        case OBJ_NATIVE:
            printf("<native fn>");
            break;
    }
}

bool objsEqual(Obj* a, Obj* b)
{
    if (a->type != b->type) return false;

    switch (a->type)
    {
        case OBJ_STRING: {
            ObjString* aString = (ObjString*)a;
            ObjString* bString = (ObjString*)b;
            return aString->length == bString->length &&
                strncmp(aString->chars, bString->chars, aString->length) == 0;
        }
        case OBJ_FUNCTION:
            return false;
            break;
        case OBJ_NATIVE: {
            ObjNative* aNative = (ObjNative*)a;
            ObjNative* bNative = (ObjNative*)b;
            return  aNative->function == bNative->function;
        }
    }   
}
