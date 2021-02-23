#include "disassembler.hpp"
#include "chunk.hpp"
#include <iomanip> 
#include <iostream>
#include <string>

int Disassembler::simpleInstruction(std::string const name, int offset)
{
    std::cout << offset << ": " << name << '\n';
    return offset + 1;
}

int Disassembler::constantInstruction(std::string const name, int offset)
{
    int constant_index = static_cast<int>(chunk_.code[offset + 1]);
    double constant = chunk_.constants.values[constant_index];
    std::cout << offset << ": " << name << " [" << constant_index << "] - " << constant << '\n';
    return offset + 2;
}

void Disassembler::disassemble(std::string const name)
{
    std::cout << " == " << name << "==\n";

    for (int offset = 0; offset < chunk_.code.size();)
    {
        offset = disassembleInstruction(offset);
    }
}

int Disassembler::disassembleInstruction(int offset)
{
    OpCode instruction = static_cast<OpCode>(chunk_.code[offset]);

    int line = chunk_.lines[offset];

    std::cout << std::setfill('0') << std::setw(4) << offset << " ";
    if (offset > 0 && line == chunk_.lines[offset - 1])
    {
    	std::cout << "| ";
    }
    else
    {
    	std::cout << line << " ";
    }

    switch (instruction)
    {
        case OpCode::OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OpCode::OP_CONSTANT:
            return constantInstruction("OP_CONSTANT", offset);
        default:
            std::cout << "Unknow opcode\n";
            return offset + 1;
    }
}
