#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

void return_statement::execute()
{	
	//return void
	if (expression == nullptr) {
		owner->return_value = nullptr;
		return;
	}

	std::unique_ptr<operand> value = evaluate_expression(owner, expression->ast_tree);
	
	const auto result = datatype::create_type<integer_dt, int>(*value->get_value());
	const auto expression_condition = result.get();

	std::cout << "result: " << expression_condition << '\n';

	owner->return_value = value.release();
}