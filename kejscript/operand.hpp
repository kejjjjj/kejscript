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
	bool has_value()  const noexcept;
	bool is_integral() const noexcept;
	bool is_numeric() const noexcept;
	bool bool_convertible() const noexcept;

	bool is_compatible_with(const operand& other) const;

	datatype* lvalue_to_rvalue();
	datatype* get_value();
	datatype* get_value() const;

	token_t* _operand = nullptr;

	std::shared_ptr<string_object> string;

	std::shared_ptr<object> is_object() const noexcept
	{

		if (type == Type::LVALUE) {
			auto& var = std::get<std::shared_ptr<variable>>(value);

			if (var->obj)
				return var->obj;
		}
		return nullptr;
	}

	std::shared_ptr<string_object> is_string() const noexcept
	{
		if (string)
			return string;

		if (type == Type::LVALUE) {
			auto& var = std::get<std::shared_ptr<variable>>(value);

			if (var->string)
				return var->string;
		}

		return nullptr;

	}
	bool is_string_literal()
	{
		return !is_object() && type == Type::RVALUE && get_value()->type() == datatype_e::string_t;
	}

	std::string get_type() const noexcept
	{
		if (is_object())
			return "object";

		return get_value()->type_str();
	}

	//std::shared_ptr<object> obj;

private:
	NO_COPY_CONSTRUCTOR(operand);


};

struct object
{
	object() = default;
	//~object() { LOG("~object\n"); }
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

struct string_object
{
	string_object() = default;

	void insert(char c) {

		variables.push_back(std::make_shared<variable>());
		auto& back = variables.back();

		back->immutable = true;
		back->value = std::make_unique<char_dt>(c);
		back->identifier = "character";
		back->initialized = true;
	}

	std::string get_string() const noexcept 
	{
		std::string str;

		for (auto& v : variables)
			str.push_back(v->value->getvalue<char_dt>().get());

		return str;
	}
	//std::shared_ptr<string_dt> raw_data;
	//std::shared_ptr<variable> owner;
	std::vector<std::shared_ptr<variable>> variables;
	//std::string actual_string;
	NO_COPY_CONSTRUCTOR(string_object);

};