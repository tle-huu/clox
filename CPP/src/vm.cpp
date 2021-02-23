#include "vm.hpp"

#include <iostream>

#define BINARY_OP(op) \
    do { \
      double b = pop(); \
      double a = pop(); \
      push(a op b); \
    } while (false)

InterpretResult VM::run()
{
    if (chunk_ == nullptr)
    {
        std::cout << "ERROR: vm has no loaded chunk.\n";
        return InterpretResult::COMPILE_ERROR;
    }

    while (true)
    {
        OpCode instruction = static_cast<OpCode>(read_byte());
        
        switch (instruction)
        {
            case OpCode::OP_CONSTANT:
            {
                Value constant = read_constant();
                push(constant);
                break;
            }
            case OpCode::OP_NEGATE:
            {
                push(-pop());
                break;                
            }
            case OpCode::OP_ADD:        BINARY_OP(+); break;
            case OpCode::OP_SUBSTRACT:  BINARY_OP(-); break;
            case OpCode::OP_MULTIPLY:   BINARY_OP(*); break;
            case OpCode::OP_DIVIDE:     BINARY_OP(/); break;
            case OpCode::OP_RETURN:
                std::cout << "value: " << pop() << '\n';
                return InterpretResult::OK;
            default:
                return InterpretResult::RUNTIME_ERROR;
        }
    }
    return InterpretResult::OK;
}

InterpretResult VM::interpret(std::string const & source)
{
    Chunk chunk;

    compiler_.compile(source, chunk);
    load_chunk(&chunk);

    InterpretResult result = run();
    return result;
}
