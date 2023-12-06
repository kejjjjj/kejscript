#pragma once

#include "pch.hpp"


struct expression_t
{
	std::vector<token_t*> prefix;
	token_t* identifier = 0;
	std::vector<token_t*> postfix;
	
	struct operator_t{
		bool is_operator = false;
		punctuation_e punc = punctuation_e::P_UNKNOWN;
		OperatorPriority priority = OperatorPriority::FAILURE;
	}op;

	bool empty() const noexcept {
		return prefix.empty() && identifier == nullptr;
	}
};

struct expression_context
{
	expression_context(const expression_token_stack& _stack) : stack(_stack) {}

	expression_t expression;
	std::vector<expression_t> expressions;
	expression_token_stack stack;
	
	expression_context(const expression_context&) = delete;
	expression_context& operator=(const expression_context&) = delete;

};

[[nodiscard]] VectorTokenPtr::iterator evaluate_expression(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, const expression_token_stack & = expression_token_stack());
