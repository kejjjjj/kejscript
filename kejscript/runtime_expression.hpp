#pragma once

#include "pch.hpp"
#include "operators.hpp"

struct expression_context
{
	expression_context(const expression_token_stack& _stack) : stack(_stack) {}

	expression_t expression;
	std::vector<expression_t> expressions;
	expression_token_stack stack;
	
	expression_context(const expression_context&) = delete;
	expression_context& operator=(const expression_context&) = delete;

};

struct expression_results
{
	expression_results() = default;
	expression_results(auto& _it, auto& expr): it(_it), expression(std::move(expr)){}
	expression_results(auto& _it) : it(_it), expression(nullptr) {}

	VectorTokenPtr::iterator it;
	std::unique_ptr<expression_node> expression;

	expression_results& operator=(const expression_results&) = delete;
	expression_results(const expression_results&) = delete;
};

[[nodiscard]] std::unique_ptr<expression_results> evaluate_expression(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, const expression_token_stack & = expression_token_stack());
