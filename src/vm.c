#include "common.h"
#include "compiler.h"
#include "memory.h"
#include "object.h"
#include "vm.h"
#include "value.h"

#ifdef DEBUG_TRACE_EXECUTION
#include "debug.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

VM vm;

static Value clockNative(int argCount, Value* args)
{
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void resetStack()
{
    vm.stackTop = vm.stack;
    vm.frameCount = 0;
}

static void runtimeError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);

    for (int i = vm.frameCount - 1; i >= 0; --i)
    {
        CallFrame* frame = &vm.frames[i];
        ObjFunction* function = frame->function;

        size_t instruction = frame->ip - function->chunk.code - 1;
        int line = getLine(&frame->function->chunk.lines, instruction).line;
        fprintf(stderr, "[line %d] in ", line);

        if (function->name == NULL)
        {
            fprintf(stderr, "script\n");
        }
        else
        {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }

    resetStack();
}

static Value peek(int distance)
{
    return vm.stackTop[-1 - distance];
}

static void defineNative(const char* name, NativeFn function, int arity)
{
    push(OBJ_VAL(copyString(name, (int)strlen(name))));
    push(OBJ_VAL(newNative(function, arity)));
    tableSet(&vm.globals, AS_STRING(peek(1)), peek(0));
    pop();
    pop();
}

void initVM()
{
    resetStack();
    initTable(&vm.strings);
    initTable(&vm.globals);
    vm.objects = NULL;

    defineNative("clock", clockNative, 0);
}

void freeVM()
{
    freeTable(&vm.strings);
    freeObjects();
}

void push(Value value)
{
    if (vm.stackTop == vm.stack + STACK_MAX - 1)
    {
        runtimeError("FATAL: Stack overflow.");
        exit(42);
    }
    *vm.stackTop = value;
    ++vm.stackTop;

}

Value pop()
{
    vm.stackTop--;
    return *vm.stackTop;
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

static bool call(ObjFunction* function, uint8_t argCount)
{
    if (argCount != function->arity)
    {
        runtimeError("Expected %d arguments but %d were given.", function->arity, argCount);
        return false;
    }

    if (vm.frameCount == FRAMES_MAX)
    {
        runtimeError("Stack overflow.");
        return false;
    }
    CallFrame* frame = &vm.frames[vm.frameCount++];

    frame->function = function;
    frame->ip = function->chunk.code;

    frame->slots = vm.stackTop - argCount - 1;
    return true;
}

static bool callValue(Value callee, int argCount)
{
    if (IS_OBJ(callee)) {

        switch (OBJ_TYPE(callee))
        {
            case OBJ_FUNCTION:
                return call(AS_FUNCTION(callee), argCount);
            case OBJ_NATIVE: {
                int arity = ((ObjNative*)AS_OBJ(callee))->arity;
                NativeFn native = AS_NATIVE(callee);
                if (argCount != arity)
                {
                    runtimeError("Expect %d arguments but %d were given\n", arity, argCount);
                    return false;
                }
                Value result = native(argCount, vm.stackTop - argCount);

                if (IS_NATIVE_ERROR(result))
                {
                    runtimeError(AS_CSTRING(result));
                    return false;
                }
                vm.stackTop -= argCount + 1;
                push(result);
                return true;
            }
            case OBJ_STRING:
            default:
                break;
        }
    }

    runtimeError("Object is not callable.");
    return false;
}

static InterpretResult run()
{
    CallFrame* frame = &vm.frames[vm.frameCount - 1];

    register uint8_t* instruction_pointer = frame->ip;

#define READ_BYTE() (*instruction_pointer++)
#define READ_CONSTANT() (frame->function->chunk.constants.values[READ_BYTE()])
#define READ_STRING() AS_STRING(READ_CONSTANT())
#define READ_SHORT()\
    (instruction_pointer += 2, (uint16_t)((instruction_pointer[-2] << 8) | instruction_pointer[-1]))
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
#define RESTORE_IP() frame->ip = instruction_pointer


    while (true)
    {
        // sleep(1);
    #ifdef DEBUG_TRACE_EXECUTION
        printf("          ");
        for (Value* slot = vm.stack; slot < vm.stackTop; ++slot)
        {
            printf("[ ");
            printValue(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(&frame->function->chunk, (int)(instruction_pointer - frame->function->chunk.code));
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
                    RESTORE_IP();
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
                    RESTORE_IP();
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
                break;
            case OP_DEFINE_GLOBAL: {
                ObjString* name = READ_STRING();
                tableSet(&vm.globals, name, peek(0));
                pop();
                break;
            }
            case OP_SET_GLOBAL: {
                ObjString* name = READ_STRING();
                if (tableSet(&vm.globals, name, peek(0))) {
                    tableDelete(&vm.globals, name);
                    runtimeError("Undefined global variable '%s'.", name->chars);
                    RESTORE_IP();
                    return INTERPRET_RUNTIME_ERROR;
                }
                break;
            }
            case OP_GET_GLOBAL: {
                ObjString* name = READ_STRING();
                Value value;
                bool exists = tableGet(&vm.globals, name, &value);
                if (!exists) {
                    runtimeError("Undefined global variable '%s'.", name->chars);
                    RESTORE_IP();
                    return INTERPRET_RUNTIME_ERROR;
                }
                push(value);
                break;

            }
            case OP_GET_LOCAL: {
                uint8_t slot = READ_BYTE();
                push(frame->slots[slot]);
                break;
            }
            case OP_SET_LOCAL: {
                uint8_t slot = READ_BYTE();
                frame->slots[slot] = peek(0);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                if (isFalsey(peek(0))) instruction_pointer += offset;
                break;
            }
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                instruction_pointer += offset;
                break;   
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                instruction_pointer -= offset;
                break;
            }
            case OP_CALL: {
                int argCount = READ_BYTE();

                if (!callValue(peek(argCount), argCount)) {
                    RESTORE_IP();
                    return INTERPRET_RUNTIME_ERROR;
                }
                RESTORE_IP();
                frame = &vm.frames[vm.frameCount - 1];
                instruction_pointer = frame->ip;
                break;
            }
            case OP_RETURN   : {
                Value result = pop();
                vm.frameCount--;

                if (vm.frameCount == 0)
                {
                    pop();
                    RESTORE_IP();
                    return INTERPRET_OK;
                }

                vm.stackTop = frame->slots;
                push(result);

                // RESTORE_IP();
                frame = &vm.frames[vm.frameCount - 1];
                instruction_pointer = frame->ip;
                break;
            }
            default:
                printf("Unknown op code: [%u]\n", instruction);
                break;
               
        }
    }
    RESTORE_IP();
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef READ_SHORT
#undef BINARY_OP
}

InterpretResult interpret(const char* source)
{
    // Compiling
    clock_t begin = clock();
    ObjFunction* function = compile(source);

    if (function == NULL) return INTERPRET_COMPILE_ERROR;

    clock_t end = clock();
    printf("Compile time: %f seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);

    // Setting function and call frame
    push(OBJ_VAL(function));
    callValue(OBJ_VAL(function), 0);

    // Interpreting
    begin = clock();
    InterpretResult result = run();
    end = clock();
    printf("Run time: %f seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);
    return result;
}
