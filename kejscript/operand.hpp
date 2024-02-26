#pragma once

#include "pch.hpp"
#include "variable.hpp"

struct operand;

using shared_datatype = std::shared_ptr<datatype>;

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

	operand() /*: value(std::make_shared<variable>()), type(Type::LVALUE)*/
	{
		
	};
	operand(singular& expr, struct function_stack* stack);
	operand(std::shared_ptr<variable>& v) : value(v), type(Type::LVALUE) {}

	//template<typename T>
	//operand(const T& expr)
	//{
	//	make_rvalue<T>(expr);
	//}

	type_value rvalue;

	std::shared_ptr<variable> value;
	void cast_weaker_operand(datatype_e this_type, datatype_e other_type, operand& other);
	void cast_weaker_operand(type_value* other_type);

	void implicit_cast(operand& other);
	void implicit_cast(type_value* other);

	void implicit_cast(operand& other, type_value* l, type_value* r);
	bool has_value()  const noexcept;
	bool is_integral() const noexcept;
	bool is_numeric() const noexcept;
	bool bool_convertible() const noexcept;
	bool valueless_type() const noexcept;

	void make_lvalue(std::shared_ptr<variable>& v) { value = v; type = Type::LVALUE; }

	template<typename T>
	void make_rvalue(const T& v) 
	{ 
		rvalue.value = (v); 
		rvalue.rvalue_t = v.type(); 
		rvalue.data = &rvalue.get<T>().value;
		type = Type::RVALUE; 
	}



	bool is_compatible_with(const operand& other) const;

	type_value& lvalue_to_rvalue();
	
	type_value* get_value();
	type_value* get_value() const;

	token_t* _operand = nullptr;

	std::shared_ptr<string_object> string;

	auto& get_lvalue() const {
		return value;
	}
	auto& get_lvalue() {
		return value;
	}
	std::shared_ptr<object> is_object() const noexcept
	{

		if (type == Type::LVALUE) {
			auto& var = value;

			if (var && var->obj)
				return var->obj;
		}
		return nullptr;
	}

	std::shared_ptr<string_object> is_string() const noexcept
	{
		if (string)
			return string;

		if (type == Type::LVALUE) {
			auto& var = value;

			if (var && var->string)
				return var->string;
		}

		return nullptr;

	}

	function_block* is_function_pointer() const noexcept 
	{
		if (type != Type::LVALUE)
			return nullptr;

		auto& var = value;

		if (var && var->function_pointer)
			return var->function_pointer;

		return nullptr;
	}

	bool is_string_literal()
	{
		return !is_object() && !is_function_pointer() && type == Type::RVALUE && get_value()->rvalue_t == datatype_e::string_t;
	}

	std::string get_type() const noexcept
	{
		if (is_string())
			return "string";

		if (is_object())
			return "object";

		if (is_function_pointer())
			return "fptr";

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

	void insert(operand* ptr, const std::string& identifier="") {
		auto obj = ptr->is_object();

		variables.push_back(std::make_shared<variable>(identifier));
		auto& back = variables.back();

		if (auto p = ptr->is_function_pointer()) {
			back->function_pointer = p;
			back->initialized = true;
		}
		else if (!obj) {
			ptr->lvalue_to_rvalue();
			back->get_value() = *ptr->get_value();
			back->initialized = true;
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
		back->set_value(char_dt(c));
		back->identifier = "character";
		back->initialized = true;
	}

	std::string get_string() const noexcept 
	{
		std::string str;

		for (auto& v : variables)
			str.push_back(v->get_value().get<char_dt>().get());

		return str;
	}
	//std::shared_ptr<string_dt> raw_data;
	//std::shared_ptr<variable> owner;
	std::vector<std::shared_ptr<variable>> variables;
	//std::string actual_string;
	NO_COPY_CONSTRUCTOR(string_object);

};