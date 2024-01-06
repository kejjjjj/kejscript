#include "pch.hpp"

#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "operators.hpp"
#include "function.hpp"

bool expression_block::execute(function_stack* stack)
{
	evaluate_expression(owner, stack, ast_tree);
	return false;
}

std::unique_ptr<operand> evaluate_expression(function_block* owner, function_stack* stack, const nodeptr& node)
{

	if (node->is_leaf()) {

		//if it is a function call:
		auto& function = node->contents->callable;

		if (function) {
			expression_block* arg = function->arguments.get();
			//LOG("calling " << function->target << " from " << owner << '\n');
			std::list<std::unique_ptr<operand>> args;

			while (arg) {

				args.push_back(evaluate_expression(owner, stack, arg->ast_tree));
				arg = arg->next.get();
			}

			return call_function(owner, function->target, args, stack);
		}		

		return std::make_unique<operand>(*node->contents.get(), stack);
	}

	auto left_operand = evaluate_expression(owner, stack, node->left);
	auto right_operand = evaluate_expression(owner, stack, node->right);

	//if (left_operand->has_value() == false) {
	//	throw runtime_error(left_operand->_operand, "left operand does not have a value");
	//}else if (right_operand->has_value() == false) {
	//	throw runtime_error(right_operand->_operand, "right operand does not have a value");
	//}

	//std::cout << "evaluating: " << right_operand->get_value()->value.size() << '\n';


	auto& op = std::get<_operator>(node->contents->value);
	return reinterpret_cast<evaluation_functions::funcptr>(op.eval)(*left_operand, *right_operand);
}
