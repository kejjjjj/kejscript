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
		RVALUE
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

	auto& get_lvalue() const noexcept {
		return std::get<std::shared_ptr<variable>>(value);
	}

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

	function_block* is_function_pointer() const noexcept 
	{
		if (type != Type::LVALUE)
			return nullptr;

		auto& var = std::get<std::shared_ptr<variable>>(value);
		return var->function_pointer;

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

	void insert(operand_ptr& ptr, const std::string& identifier="") {
		auto obj = ptr->is_object();

		variables.push_back(std::make_shared<variable>(identifier));
		auto& back = variables.back();

		if (auto p = ptr->is_function_pointer()) {
			back->function_pointer = p;
			back->initialized = true;
		}
		else if (!obj) {
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
	void insert(const std::string& identifier="")
	{
		variables.push_back(std::make_shared<variable>(identifier));
		auto& back = variables.back();
	
		back->initialized = false;

	}
	struct_def* structure = 0;
	std::vector<std::shared_ptr<variable>> variables;
	bool is_struct = false;
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