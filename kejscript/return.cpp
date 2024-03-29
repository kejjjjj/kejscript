#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

bool return_statement::execute(function_stack* stack)
{	
	//return void
	if (expression == nullptr) {
		//owner->return_value = nullptr;
		return true;
	}

	evaluate_expression(owner, stack, expression->expression_ast);
	
	//const auto result = datatype::create_type<integer_dt, int>(*value->get_value());
	//const auto expression_condition = result.get();

	//std::cout << "result: " << expression_condition << '\n';

	//owner->return_value = value.release();

	return true;
}