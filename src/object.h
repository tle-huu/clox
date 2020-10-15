#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"

#define OBJ_TYPE(value) (AS_OBJ(value)->type)
#define IS_STRING(value) isObjType(value, OBJ_STRING)
#define AS_STRING(value) ((ObjString*)AS_OBJ(value))
#define AS_CSTRING(value) (AS_STRING(value)->chars)

typedef enum {
    OBJ_STRING,
} ObjType;

struct sObj {
    ObjType type;
    struct sObj* next;
};

struct sObjString {
    Obj obj;
    uint32_t hash;
    int length;
    char chars[];
};

ObjString* copyString(const char* start, int length);
ObjString* concatenateStrings(const ObjString* a, const ObjString* b);
void printObject(Obj* object);


static inline bool isObjType(Value value, ObjType type)
{
    return IS_OBJ(value) && (AS_OBJ(value)->type == type);
}

bool objsEqual(Obj* a, Obj* b);


#endif
