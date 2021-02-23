#pragma once

#include <vector>

using Value = double;

struct ValueArray
{
	std::vector<Value> values;

	inline int count() { return values.size(); }
	void write(Value value);
};
