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
	operand() = default;
	operand(singular& expr, struct function_stack* stack);
	operand(variable* v) : value(v), type(Type::LVALUE) {}
	operand(std::unique_ptr<datatype>&& expr) : value(std::move(expr)), type(Type::RVALUE) {}

	~operand() {
		//std::cout << "~operand()\n";
	}

	std::variant<std::unique_ptr<datatype>, variable*> value;
	
	void cast_weaker_operand(datatype_e this_type, datatype_e other_type, operand& other);
	void cast_weaker_operand(datatype* other_type);

	void implicit_cast(operand& other);
	void implicit_cast(datatype* other);

	void implicit_cast(operand& other, datatype* l, datatype* r);
	bool has_value() noexcept;
	bool is_integral();
	bool bool_convertible();

	datatype* lvalue_to_rvalue();
	std::unique_ptr<operand> create_copy();
	datatype* get_value();
	std::unique_ptr<datatype>& get_value_move();

	std::vector<token_t*> prefix;
	std::vector<token_t*> postfix;
	token_t* _operand = nullptr;

private:
	NO_COPY_CONSTRUCTOR(operand);


};