#pragma once

#include "pch.hpp"

enum class scope_type_e
{
	UNKNOWN,
	IF,
	ELSE,
	WHILE
};

class linting_scope
{
public:
	linting_scope() = default;
	bool is_global_scope() const noexcept { return lower_scope == nullptr; }
	bool declare_variable(const std::string& name) {  
		if (variable_exists(name))
			return false;
		variable_table.insert(name); 
		return true;
	}
	bool variable_exists(const std::string& name) { 

		const bool found = variable_table.find(name) != variable_table.end();

		if (found)
			return true;

		if (is_global_scope())
			return false;

		return lower_scope->variable_exists(name); 
	}

	void print_stack() const noexcept {
		//LOG("----- local vars -----\n\n");
		//for (auto& v : variable_table)
		//	LOG(v << '\n');
		//LOG('\n');
	}
	bool is_function_scope() const noexcept {
		return is_inside_of_a_function;
	}
	void emit_to_lower_scope(const scope_type_e type) noexcept {
		if (lower_scope)
			lower_scope->upper_scope_type = type;
	}
	std::optional<scope_type_e> get_previous_scope_context() const noexcept {
		return upper_scope_type != scope_type_e::UNKNOWN ? std::make_optional(upper_scope_type) : std::nullopt;
	}

	linting_scope* lower_scope = 0;
	bool is_inside_of_a_function = false;
	//code_block* block = nullptr;

	scope_type_e scope_type = scope_type_e::UNKNOWN;

private:
	scope_type_e upper_scope_type = scope_type_e::UNKNOWN;
	std::unordered_set<std::string> variable_table;
	
	NO_COPY_CONSTRUCTOR(linting_scope);

};

linting_scope* linting_create_scope_without_range(linting_scope* block);
linting_scope* linting_delete_scope(ListTokenPtr::iterator& it, linting_scope* block);