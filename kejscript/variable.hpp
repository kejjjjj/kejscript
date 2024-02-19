#pragma once

#include "pch.hpp"
#include "datatype.hpp"

struct object;
struct string_object;

struct variable
{
	variable() = default;
	variable(const std::string i) : identifier(i) {};
	~variable() = default;

	std::string identifier;
	std::unique_ptr<datatype> value;
	bool initialized = false;

	std::shared_ptr<object> obj;
	std::shared_ptr<string_object> string;

	void print(size_t spaces = 0);

private:

	NO_COPY_CONSTRUCTOR(variable);

};
