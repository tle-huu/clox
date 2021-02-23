#pragma once

#include "disassembler.hpp"
#include "value.hpp"

#include <cstdint>
#include <string>
#include <vector>

enum class OpCode : uint8_t
{
    OP_CONSTANT,
    OP_NEGATE,
    OP_ADD,
    OP_SUBSTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_RETURN,
};

struct Chunk
{

    Chunk() : disassembler_(*this) {};

    std::vector<uint8_t> code;
    std::vector<int> lines; 

    ValueArray constants;

public:
    
    void write(uint8_t byte, int line);

    void disassemble(std::string const name);

    int add_constant(Value const value);


private:

    Disassembler disassembler_;

};
