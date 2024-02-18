#pragma once

#include "pch.hpp"
#include "variable.hpp"

struct operand;

using datatype_ptr = std::unique_ptr<datatype>;
using operand_ptr = std::unique_ptr<operand>;

struct object;

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
	operand(std::shared_ptr<variable> v) : value(v), type(Type::LVALUE) {}
	operand(datatype_ptr&& expr) : value(std::move(expr)), type(Type::RVALUE) {}

	std::variant<datatype_ptr, std::shared_ptr<variable>> value;
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

	std::shared_ptr<object> is_object() const noexcept
	{

		if (type == Type::LVALUE) {
			auto& var = std::get<std::shared_ptr<variable>>(value);

			if (var->obj)
				return var->obj;
		}
		return nullptr;
	}

	//std::shared_ptr<object> obj;

private:
	NO_COPY_CONSTRUCTOR(operand);


};

struct object
{
	object() = default;
	~object() { LOG("~object\n"); }
	void insert(operand_ptr& ptr) {


		auto obj = ptr->is_object();

		variables.push_back(std::make_shared<variable>());
		auto& back = variables.back();

		if (!obj) {
			ptr->lvalue_to_rvalue();
			back->value = std::move(std::get<datatype_ptr>(ptr->value));
			back->initialized = true;
			back->identifier = "list_element";

		}
		else {
			back->obj = obj;
			back->initialized = true;
		}
	}

	std::vector<std::shared_ptr<variable>> variables;
	NO_COPY_CONSTRUCTOR(object);

};