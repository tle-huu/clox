#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "vm.h"
#include "value.h"

#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

VM vm;

static void resetStack()
{
    vm.stackTop = vm.stack;
}

static const char* getGlobalName(int index)
{
    Entry* entries = parser.globals.entries;
    for (int i = 0; i < parser.globals.capacity; ++i)
    {
        if (entries[i].key != NULL && AS_INDEX(entries[i].value) == index)
        {
            return entries[i].key->chars;
        }
    }
    return NULL;
}

static void initGlobals()
{
    initValueArray(&vm.globals);
}

static bool setGlobal(int index, Value value)
{
    if (vm.globals.capacity <= index)
    {
        int oldCapacity = vm.globals.capacity;
        vm.globals.capacity = GROW_CAPACITY(index);
        vm.globals.values = GROW_ARRAY(Value, vm.globals.values, oldCapacity, vm.globals.capacity);
        for (int i = oldCapacity; i < vm.globals.capacity; ++i)
        {
            vm.globals.values[i] = NIL_VAL;
        }
    }

    bool isNew = false;
    if (IS_NIL(vm.globals.values[index]))
    {
        vm.globals.count++;
        isNew = true;
    }
    vm.globals.values[index] = value;
    return isNew;
}

static bool getGlobal(int index, Value* value)
{
    if (index >= vm.globals.capacity || IS_NIL(vm.globals.values[index])) return false;

    *value = vm.globals.values[index];
    return true;
}

static void deleteGlobal(int index)
{
    vm.globals.values[index] = NIL_VAL;
}

void initVM()
{
    resetStack();
    initTable(&vm.strings);
    // initTable(&vm.globals);
    initGlobals();
    vm.objects = NULL;
}


void freeVM()
{
    freeTable(&vm.strings);
    freeObjects();
}

void push(Value value)
{
    *vm.stackTop = value;
    ++vm.stackTop;
}

Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
}


static void runtimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    size_t instruction = vm.ip - vm.chunk->code - 1;
    int line = getLine(&vm.chunk->lines, instruction).line;
    fprintf(stderr, "[line %d] in script\n", line);

    resetStack();
}

static Value peek(int distance)
{
    return vm.stackTop[-1 -distance];
}

static bool isFalsey(Value value)
{
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate()
{
    ObjString* b = AS_STRING(pop());
    ObjString* a = AS_STRING(pop());

    ObjString* string = concatenateStrings(a, b);

    push(OBJ_VAL(string));

}

static InterpretResult run()
{
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_INDEX() AS_INDEX(READ_CONSTANT())
#define BINARY_OP(valueType, op) \
        do { \
            if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
                runtimeError("Operands must be numbers."); \
                return INTERPRET_RUNTIME_ERROR; \
            } \
            double b = AS_NUMBER(pop()); \
            double a = AS_NUMBER(pop()); \
            push(valueType(a op b)); \
        } while (false)


    while (true)
    {
    #ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; ++slot)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
    #endif
        uint8_t instruction;
        switch (instruction = READ_BYTE())
        {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                push(constant);
                break;
            }
            case OP_NEGATE   :
                if (!IS_NUMBER(peek(0)))
                {
                    runtimeError("Operand must be a number.");
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(NUMBER_VAL(-AS_NUMBER(pop())));
                break;
            case OP_ADD      : {

                if (IS_STRING(peek(0)) && IS_STRING(peek(1)))
                {
                    concatenate();
                }
                else if (IS_NUMBER(peek(0)) && IS_NUMBER(peek(1)))
                {
                    double b = AS_NUMBER(pop());
                    double a = AS_NUMBER(pop());
                    push(NUMBER_VAL(a + b));
                }
                else
                {
                    runtimeError("Operants must be two numbers or two strings.");
                    return INTERPRET_RUNTIME_ERROR;
                }

                break; 
            }

            case OP_SUBSTRACT: BINARY_OP(NUMBER_VAL, -); break;
            case OP_MULTIPLY : BINARY_OP(NUMBER_VAL, *); break;
            case OP_DIVIDE   : BINARY_OP(NUMBER_VAL, /); break;
            case OP_NIL      : push(NIL_VAL); break;
            case OP_TRUE     : push(BOOL_VAL(true)); break;
            case OP_FALSE    : push(BOOL_VAL(false)); break;
            case OP_NOT      : push(BOOL_VAL(isFalsey(pop()))); break;
            case OP_GREATER  : BINARY_OP(BOOL_VAL, >); break;
            case OP_LESS     : BINARY_OP(BOOL_VAL, <); break;
            case OP_EQUAL    : {
                Value b = pop();
                Value a = pop();
                push(BOOL_VAL(valuesEqual(a, b)));
                break;
            }
            case OP_POP      : pop(); break;
            case OP_PRINT:
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            case OP_DEFINE_GLOBAL: {
                int index = READ_INDEX();
                setGlobal(index, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                int index = READ_INDEX();
                if (setGlobal(index, peek(0)))
                {
                    deleteGlobal(index);
                    runtimeError("Undefined global variable '%s'.", getGlobalName(index));
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_GLOBAL: {
                int index = READ_INDEX();
                Value value;
                bool exists = getGlobal(index, &value);
                if (!exists) {
                    runtimeError("Undefined global variable '%s'.", getGlobalName(index));
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;

            }
            case OP_RETURN   :
                // Exit interpreter
                return INTERPRET_OK;
               
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef BINARY_OP
}

InterpretResult interpret(const char* source)
{
    Chunk chunk;
    initChunk(&chunk);

    clock_t begin = clock();
    if (!compile(source, &chunk))
    {
        return INTERPRET_COMPILE_ERROR;
    }
    clock_t end = clock();
    printf("Compile time: %f seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;

    begin = clock();
    InterpretResult result = run();
    end = clock();
    printf("Run time: %f seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);
    freeChunk(&chunk);
    return result;
}
