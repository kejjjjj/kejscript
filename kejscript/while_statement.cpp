#include "pch.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

bool while_block::execute(function_stack* stack)
{
	
	while (true) {
		const auto value = evaluate_expression(owner, stack, condition->expression_ast);
		if (value->bool_convertible() == false)
			throw runtime_error(value->_operand, "the expression must be convertible to a boolean type");

		bool expression_condition = *reinterpret_cast<bool*>(value->get_value()->value.data());

		if (expression_condition == false)
			break;

		if (eval_block(stack))
			return true;

	}

	return false;
}