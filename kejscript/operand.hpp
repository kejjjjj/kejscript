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

	operand(singular& expr);
	operand(variable* v) : value(v), type(Type::LVALUE) {}
	operand(std::unique_ptr<datatype>&& expr) : value(std::move(expr)), type(Type::RVALUE) {}

	~operand() {
		//std::cout << "~operand()\n";
	}

	std::variant<std::unique_ptr<datatype>, variable*> value;
	
	void cast_weaker_operand(datatype_e this_type, datatype_e other_type, operand& other);
	void implicit_cast(operand& other);
	void implicit_cast(operand& other, datatype* l, datatype* r);

	datatype* lvalue_to_rvalue();
	datatype* get_value();
	std::vector<token_t*> prefix;
	std::vector<token_t*> postfix;
	token_t* _operand = nullptr;

private:
	NO_COPY_CONSTRUCTOR(operand);


};