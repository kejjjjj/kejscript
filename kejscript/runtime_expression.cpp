#include "pch.hpp"

#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "operators.hpp"
#include "function.hpp"

void expression_block::execute()
{
	evaluate_expression(owner, ast_tree);
}

std::unique_ptr<operand> evaluate_expression(function_block* owner, const nodeptr& node)
{
	if (node->is_leaf()) {

		//if it is a function call:
		auto& function = node->contents->callable;

		if (function) {
			expression_block* arg = function->arguments.get();
			return call_function(owner, function->target, arg);
		}

		return std::make_unique<operand>(*node->contents.get());
	}

	auto left_operand = evaluate_expression(owner, node->left);
	auto right_operand = evaluate_expression(owner, node->right);
	auto& op = std::get<_operator>(node->contents->value);
	return reinterpret_cast<evaluation_functions::funcptr>(op.eval)(*left_operand, *right_operand);
}
