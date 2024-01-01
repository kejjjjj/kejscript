#include "pch.hpp"

#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "operators.hpp"

void expression_block::execute()
{
	evaluate_expression(ast_tree);
}

std::unique_ptr<operand> evaluate_expression(const nodeptr& node)
{
	if (node->is_leaf()) {
		return std::make_unique<operand>(*node->contents.get());
	}

	auto left_operand = evaluate_expression(node->left);
	auto right_operand = evaluate_expression(node->right);
	auto& op = std::get<_operator>(node->contents->value);
	return reinterpret_cast<evaluation_functions::funcptr>(op.eval)(*left_operand, *right_operand);
}
