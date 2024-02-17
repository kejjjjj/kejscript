#pragma once

#include "pch.hpp"
#include "variable.hpp"

struct operand;

using datatype_ptr = std::unique_ptr<datatype>;
using operand_ptr = std::unique_ptr<operand>;

struct operand
{
	enum class Type
	{
		LVALUE,
		RVALUE,
		RVALUE_ARRAY
	}type = Type::LVALUE;
	operand() = default;
	operand(singular& expr, struct function_stack* stack);
	operand(variable* v) : value(v), type(Type::LVALUE) {}
	operand(datatype_ptr&& expr) : value(std::move(expr)), type(Type::RVALUE) {}

	void insert_element(operand_ptr& ptr) {
		std::get<std::vector<operand_ptr>>(value).push_back(std::move(ptr));
	}

	std::variant<datatype_ptr, variable*, std::vector<operand_ptr>> value;
	void make_array();
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
	token_t* _operand = nullptr;

private:
	NO_COPY_CONSTRUCTOR(operand);


};