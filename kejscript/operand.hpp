#pragma once

#include "pch.hpp"

#include "datatype.hpp"
#include "runtime_expression.hpp"


struct operand
{
	operand(const expression_t& expr);
	~operand() {
		std::cout << "~operand()\n";
	}

	std::unique_ptr<datatype> value;


private:
	operand& operator=(const operand&) = delete;
	operand(const operand&) = delete;
};