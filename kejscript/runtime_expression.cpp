#include "pch.hpp"

#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "operators.hpp"


[[maybe_unused]] std::unique_ptr<operand> evaluate_expression(const nodeptr& node)
{
	//a leaf
	if (node->is_leaf()) {
		return std::make_unique<operand>(*node->contents.get());
	}

	auto left_operand = evaluate_expression(node->left);
	auto right_operand = evaluate_expression(node->right);

	auto& op = std::get<_operator>(node->contents->value);

	auto function = evaluation_functions::getInstance().find_function(op.punc);

	if (function.has_value() == false)
		throw runtime_error("unsupported operator");

	auto v =  function->operator()(*left_operand, *right_operand);
	std::cout << "evaluated!\n";
	return v;
}
