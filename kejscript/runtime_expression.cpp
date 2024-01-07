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
void evaluate_unary(operand& target, const nodeptr& node)
{
	if (node->is_unary() == false)
		return;

	auto& op = node->get_operator();

	reinterpret_cast<evaluation_functions::unaryfuncptr>(op.eval)(target);
	return evaluate_unary(target, node->left);

}
std::unique_ptr<operand> evaluate_expression(function_block* owner, function_stack* stack, const nodeptr& node)
{
	if (node->is_unary()) {
		auto expression = evaluate_expression(owner, stack, node->left);
		evaluate_unary(*expression.get(), node);
		return std::move(expression);
	}

	if (node->is_leaf()) {

		//if it is a function call:
		auto& leaf = node->get_operand();
		auto& function = leaf->callable;

		if (function) {
			expression_block* arg = function->arguments.get();
			std::list<std::unique_ptr<operand>> args;
			while (arg) {
				args.push_back(evaluate_expression(owner, stack, arg->ast_tree));
				arg = arg->next.get();
			}
			return call_function(owner, function->target, args, stack);
		}		

		return std::make_unique<operand>(*leaf.get(), stack);
	}

	auto left_operand = evaluate_expression(owner, stack, node->left);
	auto right_operand = evaluate_expression(owner, stack, node->right);

	//if (left_operand->has_value() == false) {
	//	throw runtime_error(left_operand->_operand, "left operand does not have a value");
	//}else if (right_operand->has_value() == false) {
	//	throw runtime_error(right_operand->_operand, "right operand does not have a value");
	//}

	auto& op = node->get_operator();
	auto ret = reinterpret_cast<evaluation_functions::funcptr>(op.eval)(*left_operand, *right_operand);
	ret->_operand = right_operand->_operand;
	return ret;
}
