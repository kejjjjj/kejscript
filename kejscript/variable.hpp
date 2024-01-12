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
	void make_reference(variable* target) { b_is_reference = true; value = target; }
	void insert_element(struct operand* val);
	std::vector<std::unique_ptr<variable>> arrayElements;

	bool is_reference() const noexcept { return b_is_reference; }

	void print(size_t spaces = 0);

private:
	bool b_is_reference = false;
	variable& operator=(const variable&) = delete;
	variable(const variable&) = delete;
};
