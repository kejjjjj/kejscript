#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

bool peek_unary_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& context);
bool peek_identifier(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& context);
bool peek_postfix_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& context);

void tokenize_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& context)
{
	while (peek_unary_operator(it, end, context));
	if (peek_identifier(it, end, context) == false)
		throw linting_error(it->get(), "expected an identifier instead of '%s'", it->get()->string.c_str());
	while (peek_postfix_operator(it, end, context));

	if (context.stack.time_to_exit() && context.expression.empty())
		return;

	context.expression.op = false;
	context.expressions.push_back(context.expression);
	context.expression = linting_expression();

}
void tokenize_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& ctx)
{
	if (it == end || it->get()->is_operator(punctuation_e::P_SEMICOLON))
		return;

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false) {
		ctx.stack.assign_to_stack_if_possible(it);

		if (ctx.stack.time_to_exit())
			return;
	}

	const auto token = it->get();

	if(token->is_punctuation() == false)
		throw linting_error(token, "expected the expression to end before '%s'", token->string.c_str());

	auto op = dynamic_cast<punctuation_token_t*>(token);

	if (!satisfies_operator(op->punc)) {
		throw linting_error(token, "expected the expression to end before '%s'", token->string.c_str());
	}

	linting_expression expression;
	expression.op = true;
	expression.identifier = token;
	ctx.expressions.push_back(expression);
	std::advance(it, 1);
	return;

}
[[nodiscard]] l_expression_results evaluate_expression_sanity(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, const expression_token_stack& stack)
{
	if (it == end)
		throw linting_error(it->get(), "an empty expression is not allowed");

	l_expression_context ctx(stack);

	//ctx.stack.num_evaluations;

	while (it != end) {

		tokenize_operand(it, end, ctx);

		

		if (ctx.undefined_var) {
			if (ctx.undefined_var->function == false)
				throw linting_error(ctx.undefined_var->location->get(), "'%s' is undefined", ctx.undefined_var->identifier.c_str());
			else
				linting_data::getInstance().undefined_variables.push_back(*ctx.undefined_var.get());
		}

		//the comma should only exit if we are not using the stack
		if (it->get()->is_operator(punctuation_e::P_COMMA)) {
			
			//this condition means that we are currently parsing function arguments or we come across a random comma in the middle of parentheses
			if (ctx.stack.not_in_use() == false && ctx.stack.opening == P_PAR_OPEN)
			{
				++it;
				ctx.stack.num_evaluations += 1;
				return evaluate_expression_sanity(it, end, ctx.stack);
			}

			if (ctx.stack.num_evaluations == 0)
				throw linting_error(it->get(), "the ',' operator is not allowed here");

			break;
		}

		if (ctx.stack.time_to_exit()) {
			it = ctx.stack.location;
			LOG("returning to " << it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");
			break;
		}

		tokenize_operator(it, end, ctx);

		if (it->get()->is_operator(punctuation_e::P_SEMICOLON)) {
			break;
		}
	}

	if (ctx.expressions.empty()) {
		throw linting_error(it->get(), "an empty expression is not allowed");
	}

	if (ctx.expressions.back().op) {
		throw linting_error(it->get(), "an expression must not end at an operator");
	}

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false)
		throw linting_error(it->get(), "expected to find a '%s'\n", punctuations[ctx.stack.closing].identifier.c_str());

	//LOG("there are " << ctx.expressions.size() << " parts to this expression\n");
	
	return { it, ctx.stack.num_evaluations + 1 };
}

bool peek_unary_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& context)
{
	if (it == end)
		return false;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return false;

	if (it->get()->is_punctuation() == false)
		return false;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());
	
	if (ptr->is_operator(P_PAR_OPEN)) //parentheses are the operand, so go to next function
		return false;

	if (bad_unary_operator(ptr->punc))
		throw linting_error(it->get(), "expected an expression instead of '%s'", ptr->string.c_str());

	if (!is_unary_operator(ptr->punc)) {
		return false;
	}
	context.expression.prefix.push_back(it->get());

	std::advance(it, 1);
	return true;
}
bool peek_identifier(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& context)
{
	if (it == end)
		return false;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit()) {
		if (context.expression.prefix.empty() == false)
			throw linting_error(it->get(), "I don't think we should exit");
		return true;
	}

	if (it->get()->is_punctuation()) {
		if (it->get()->is_operator(P_PAR_OPEN)) {

			std::advance(it, 1);

			expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
			l_expression_results results = evaluate_expression_sanity(it, end, stack);

			if (results.num_evaluations > 1) {
				throw linting_error(results.it->get(), "expected one expression instead of %u", (unsigned __int64)results.num_evaluations);
			}

			it = results.it;

			context.stack.stack.num_close++;
			context.stack.location = it;

			std::advance(it, 1);
			return true;
		}
		return false;
	}
	
	auto& expression = context.expression;

	expression.identifier = it->get();

	//test if it's an identifier
	if (expression.identifier->tt == tokentype_t::IDENTIFIER) {

		//make sure it exists in the stack before it's used
		auto scope = linting_data::getInstance().active_scope;

		if (scope->variable_exists(expression.identifier->string) == false) {

			context.undefined_var = std::move(std::make_unique<undefined_variable>(
				undefined_variable{ .identifier = expression.identifier->string, .location = it, .num_args = 0 }));

			//throw linting_error(expression.identifier, "the identifier '%s' is undefined", expression.identifier->string.c_str());
		}
	}

	if (expression.identifier->is_reserved_keyword())
		throw linting_error(expression.identifier, "the '%s' keyword is a reserved keyword and it cannot be used here", expression.identifier->string.c_str());


	std::advance(it, 1);
	return true;
}
bool peek_postfix_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, l_expression_context& context)
{
	if (it == end)
		return false;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return false;

	if (it->get()->is_punctuation() == false)
		return false;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());
	
	if (!is_postfix_operator(ptr->punc))
		return false;

	context.expression.postfix.push_back(it->get());

	if (ptr->punc == P_BRACKET_OPEN) {
		it = evaluate_subscript_sanity(it, end, context);
	}
	else if (ptr->punc == P_PAR_OPEN) {
		it = evaluate_function_call_sanity(it, end, context);
	}
	else
		std::advance(it, 1);


	return true;
}