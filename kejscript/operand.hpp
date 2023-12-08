#pragma once

#include "pch.hpp"

#include "variable.hpp"

struct operand
{
	enum class Type
	{
		LVALUE,
		RVALUE
	}type = Type::LVALUE;

	operand(const expression_t& expr);
	operand(std::unique_ptr<datatype>&& expr) : value(std::move(expr)), type(Type::RVALUE) {}

	~operand() {
		std::cout << "~operand()\n";
	}
	union {
		std::variant<std::unique_ptr<datatype>, variable*> value;
	};

	datatype* lvalue_to_rvalue();

	std::vector<token_t*> prefix;
	std::vector<token_t*> postfix;

private:
	operand& operator=(const operand&) = delete;
	operand(const operand&) = delete;


};