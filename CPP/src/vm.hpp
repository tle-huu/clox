#pragma once

#include "chunk.hpp"
#include "compiler.hpp"

enum class InterpretResult
{
	OK,
	COMPILE_ERROR,
	RUNTIME_ERROR,
};


class VM
{

public:

	VM() : chunk_(nullptr) {};

	inline void load_chunk(Chunk const *chunk)
	{
		chunk_ = chunk;
		ip_ = 0;
	}

	InterpretResult interpret(std::string const & source);

private:
	InterpretResult run();

	inline uint8_t read_byte() noexcept
	{
		return chunk_->code[ip_++];
	}

	inline Value read_constant() noexcept
	{
		return chunk_->constants.values[read_byte()];
	}

	inline void push(Value value)
	{
		stack_.emplace_back(value);
	}

	Value pop()
	{
		Value value= stack_.back();
		stack_.pop_back();
		return value;
	}

private:

	std::vector<Value> stack_;

	Chunk const *chunk_;
	int ip_;

	Compiler compiler_;

};