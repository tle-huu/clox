#pragma once

#include "scanner.hpp"

struct Parser
{

public:
	Token current;
	Token previous;
	bool had_error = false;
	bool panic_mode;

public:
	void advance();

	void error(Token const & token, char const * message);
	void error_at_current(char const * message);

	void consume(TokenType type, char const * message);

	void init(std::string const & source);

	inline Token const & peek()
	{
		return current;
	}

private:

	Scanner scanner_;
};