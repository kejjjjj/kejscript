#pragma once

#include "pch.hpp"
#include "datatype.hpp"

struct object;
struct string_object;

struct variable
{
	variable() = default;
	variable(const std::string i) : identifier(i) {};
	~variable() = default;

	std::string identifier;
	
	bool initialized = false;

	std::shared_ptr<object> obj;
	std::shared_ptr<string_object> string;

	function_block* function_pointer = 0;
	std::shared_ptr<object> member = 0;

	bool immutable = false;

	template<typename T>
	void set_value(const T& v)
	{
		value.value = v;
		value.rvalue_t = v.type();
		value.data = &value.get<T>().value;
	}

	type_value& get_value() noexcept { return value; }

	bool valueless_type() const noexcept;
	void print(size_t spaces = 0);
	void print2() const;

private:

	type_value value; //prevent writing without the setter

	void print_internal(bool is_member = false) const;

	NO_COPY_CONSTRUCTOR(variable);

};
