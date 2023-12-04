#pragma once

#include "pch.hpp"

class compiler_scope
{
public:

	bool is_global_scope() const noexcept { return lower_scope == nullptr; }
	void declare_variable(const std::string& name) { variable_table.insert(name); }
	bool variable_exists(const std::string& name) { 

		const bool found = variable_table.find(name) != variable_table.end();

		if (found)
			return true;

		if (is_global_scope())
			return false;

		return lower_scope->variable_exists(name); 
	}

	void print_stack() const noexcept {
		LOG("----- local vars -----\n\n");
		for (auto& v : variable_table)
			LOG(v << '\n');
		LOG('\n');
	}

	compiler_scope* lower_scope = 0;

private:
	std::unordered_set<std::string> variable_table;
	std::unordered_set<std::string> function_table;

	
};

compiler_scope* compiler_create_scope_without_range(compiler_scope* block);
compiler_scope* compiler_delete_scope(script_t& script, compiler_scope* block);