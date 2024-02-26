#pragma once

#include "pch.hpp"

#include "variable.hpp"

struct operand;

struct function_stack
{
	function_stack() = default;
	std::vector<std::shared_ptr<operand>> variables;

	//std::shared_ptr<operand> return_value;

	std::shared_ptr<object> _this;

	NO_COPY_CONSTRUCTOR(function_stack);
};

using code_instruction = std::vector<std::unique_ptr<code_block>>;
using function_table_t = std::unordered_map<std::string, std::unique_ptr<function_block>>;

struct runtime
{
	runtime() = default;
	static void initialize(function_table_t& table) { function_table = std::move(table); };
	static void execute();

	static std::vector<function_block*> sorted_functions;
	static std::vector<script_literals*> sorted_literals;
	static std::vector<std::unique_ptr<operand>> literals;

private:
	
	static function_table_t function_table;
	static code_instruction instructions;

	

	NO_COPY_CONSTRUCTOR(runtime);
};