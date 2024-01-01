#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"
#include "function.hpp"
#include "runtime_exception.hpp"

function_table_t runtime::function_table;
code_instruction runtime::instructions;

void runtime::execute()
{

	auto& entry_point = function_table.find("main")->second;

	call_function(entry_point.get(), entry_point.get(), nullptr);
	
}