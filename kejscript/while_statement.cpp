#include "pch.hpp"
#include "while_statement.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

void while_block::execute()
{

	//todo: test if value has a bool convertible type
	
	while (true) {
		const auto value = evaluate_expression(condition->ast_tree);
		//const auto result = datatype::create_type<bool_dt, bool>(*value->get_value());
		//const bool expression_condition = result.get();

		bool expression_condition = *reinterpret_cast<bool*>(value->get_value()->value.data());

		if (expression_condition == false)
			break;

		eval_block();

	}


}