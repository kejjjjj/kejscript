#pragma once

#include "pch.hpp"

#include "variable.hpp"

struct function_stack
{
	function_stack() = default;
	std::vector<std::unique_ptr<variable>> variables;

	NO_COPY_CONSTRUCTOR(function_stack);
};

using code_instruction = std::vector<std::unique_ptr<code_block>>;
using function_table_t = std::unordered_map<std::string, std::unique_ptr<function_block>>;
struct runtime
{
	runtime() = default;
	static void initialize(function_table_t& table) { function_table = std::move(table); };
	static void execute();

	static std::unique_ptr<function_stack> stack; //this should not be a GLOBAL 
private:
	
	static function_table_t function_table;
	static code_instruction instructions;

	NO_COPY_CONSTRUCTOR(runtime);
};