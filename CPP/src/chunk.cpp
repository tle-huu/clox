#include "chunk.hpp"

#include <iostream>
#include <string>


int Chunk::add_constant(Value const value)
{
    constants.write(value);
    return constants.values.size() - 1;
}

void Chunk::write(uint8_t byte, int line)
{
    code.emplace_back(byte);
    lines.emplace_back(line);
}

void Chunk::disassemble(std::string const name)
{
    disassembler_.disassemble(name);
}
