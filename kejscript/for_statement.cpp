#include "pch.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

bool for_block::execute(function_stack* stack)
{

	if(initialization)
		[[maybe_unused]] const auto init = evaluate_expression(owner, stack, initialization->expression_ast);

	while (true) {

		if (condition) {
			const auto value = evaluate_expression(owner, stack, condition->expression_ast);
			if (value->bool_convertible() == false)
				throw runtime_error(value->_operand, "the expression must be convertible to a boolean type");

			bool expression_condition = *reinterpret_cast<bool*>(value->get_value()->data->data());

			if (expression_condition == false)
				break;
		}

		if (eval_block(stack))
			return true;

		if(execution)
			[[maybe_unused]] auto init = evaluate_expression(owner, stack, execution->expression_ast);
	}

	return false;
}