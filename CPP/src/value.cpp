#include "value.hpp"

void ValueArray::write(Value const value)
{
	values.emplace_back(value);
}