#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"
#include "function.hpp"
#include "if_statement.hpp"
#include "while_statement.hpp"
#include "runtime_exception.hpp"

function_table_t runtime::function_table;
code_instruction runtime::instructions;
std::unique_ptr<function_stack> runtime::stack;

void runtime::execute()
{

	auto& entry_point = function_table.find("main")->second;

	call_function(entry_point);
	
}