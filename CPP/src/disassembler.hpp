#pragma once

#include <string>

struct Chunk;

class Disassembler
{

public:

	Disassembler(Chunk const & chunk) : chunk_(chunk) {};

	void disassemble(std::string const name);


private:
	int disassembleInstruction(int offset);

    int simpleInstruction(std::string const name, int offset);
    int constantInstruction(std::string const name, int offset);
private:


	Chunk const & chunk_;

};