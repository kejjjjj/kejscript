#include "pch.hpp"

#include "runtime_expression.hpp"

void create_operand(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, expression_context& stack);
void create_operator(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, expression_context& stack);

bool assign_unary_to_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context);
void assign_identifier_to_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context);
bool assing_postfix_to_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context);

void evaluate_expression(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, const expression_token_stack& stack)
{
	expression_context ctx(stack);

	//ctx.stack.num_evaluations;

	while (it != end && it->get()->is_operator(punctuation_e::P_SEMICOLON) == false) {

		create_operand(it, end, ctx);

		if (ctx.stack.time_to_exit()) {
			it = ctx.stack.location;
			break;
		}

		create_operator(it, end, ctx);
	}


}

void create_operand(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, expression_context& ctx)
{
	while (assign_unary_to_operand(it, end, ctx));
	assign_identifier_to_operand(it, end, ctx);
	while (assing_postfix_to_operand(it, end, ctx));

	if (ctx.stack.time_to_exit() && ctx.expression.empty())
		return;

	ctx.expression.op = false;
	ctx.expressions.push_back(ctx.expression);
	ctx.expression = linting_expression();
}
void create_operator(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, expression_context& ctx)
{
	if (it->get()->is_operator(punctuation_e::P_SEMICOLON))
		return;

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false) {
		ctx.stack.assign_to_stack_if_possible(it);

		if (ctx.stack.time_to_exit())
			return;
	}

	const auto token = it->get();

	linting_expression expression;
	expression.op = true;
	expression.identifier = token;
	ctx.expressions.push_back(expression);
	std::advance(it, 1);
	return;
}
bool assign_unary_to_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context)
{
	if (it == end)
		return false;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return false;

	if (it->get()->is_punctuation() == false)
		return false;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());

	if (ptr->is_operator(P_PAR_OPEN)) //parentheses are the operand, so go to the next function
		return false;

	if (!is_unary_operator(ptr->punc)) {
		return false;
	}

	context.expression.prefix.push_back(it->get());

	std::advance(it, 1);
	return true;
}
void assign_identifier_to_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context)
{

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit()) {
		return;
	}

	auto& expression = context.expression;
	expression.identifier = it->get();
	std::advance(it, 1);

}
bool assing_postfix_to_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context)
{

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return false;

	if (it->get()->is_punctuation() == false)
		return false;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());

	if (!is_postfix_operator(ptr->punc))
		return false;

	context.expression.postfix.push_back(it->get());

	std::advance(it, 1);


	return true;
}