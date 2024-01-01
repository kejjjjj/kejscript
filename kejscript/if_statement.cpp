#include "if_statement.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

void conditional_block::execute()
{

	//todo: test if value has a bool convertible type

	//an else block if there is no condition
	if (condition == nullptr) {
		return eval_block();
	}
		
	const auto value = evaluate_expression(condition->ast_tree);
	//const auto result = datatype::create_type<bool_dt, bool>(*value->get_value());
	//const bool expression_condition = result.get();
	bool expression_condition = *reinterpret_cast<bool*>(value->get_value()->value.data());

	if (expression_condition == true) {
		return eval_block();
	}
	
	//see if there are else/else if blocks
	if (next) {
		return next->execute();
	}


}