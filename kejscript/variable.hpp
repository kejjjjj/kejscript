#pragma once

#include "pch.hpp"
#include "datatype.hpp"

struct variable
{
	variable() = default;
	variable(const std::string i) : identifier(i) {};
	~variable() = default;

	std::string identifier;
	std::variant<std::unique_ptr<datatype>, variable*> value;
	bool initialized = false;
	void insert_element(struct operand* val);
	std::vector<std::unique_ptr<variable>> arrayElements;

	void print(size_t spaces = 0);

private:
	variable& operator=(const variable&) = delete;
	variable(const variable&) = delete;
};
