#include "pch.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

bool while_block::execute(function_stack* stack)
{

	//todo: test if value has a bool convertible type
	
	while (true) {
		const auto value = evaluate_expression(owner, stack, condition->expression_ast);
		//const auto result = datatype::create_type<bool_dt, bool>(*value->get_value());
		//const bool expression_condition = result.get();
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