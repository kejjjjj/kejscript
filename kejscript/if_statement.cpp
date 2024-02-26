#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

bool conditional_block::execute(function_stack* stack)
{

	//todo: test if value has a bool convertible type

	//an else block if there is no condition
	if (condition == nullptr) {
		return eval_block(stack);
	}
		
	const auto value = evaluate_expression(owner, stack, condition->expression_ast);

	if (value->bool_convertible() == false)
		throw runtime_error(value->_operand, "the expression must be convertible to a boolean type");

	bool expression_condition = *reinterpret_cast<bool*>(value->get_value()->data->data());

	if (expression_condition == true) {
		return eval_block(stack);
	}
	
	//see if there are else/else if blocks
	if (next) {
		return next->execute(stack);
	}

	return false;
}