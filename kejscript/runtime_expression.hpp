#include "pch.hpp"

void evaluate_expression(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, const expression_token_stack& = expression_token_stack());


struct linting_expression
{
	std::vector<token_t*> prefix;
	token_t* identifier = 0;
	std::vector<token_t*> postfix;
	bool op = false;

	bool empty() const noexcept {
		return prefix.empty() && identifier == nullptr;
	}
};

struct expression_context
{
	expression_context(const expression_token_stack& _stack) : stack(_stack) {}

	linting_expression expression;
	std::vector<linting_expression> expressions;
	expression_token_stack stack;
	
	expression_context(const expression_context&) = delete;
	expression_context& operator=(const expression_context&) = delete;

};
