#include "pch.hpp"

struct compiler_expression
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
	compiler_expression expression;
	std::vector<compiler_expression> expressions;
	expression_token_stack stack;

};
bool peek_unary_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context);
bool peek_identifier(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context);
bool peek_postfix_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context);

void tokenize_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context)
{
	while (peek_unary_operator(it, end, context));
	if (peek_identifier(it, end, context) == false)
		throw compile_error(it->get(), "expected an identifier instead of '%s'", it->get()->string.c_str());
	while (peek_postfix_operator(it, end, context));

	if (context.stack.time_to_exit() && context.expression.empty())
		return;

	context.expression.op = false;
	context.expressions.push_back(context.expression);
	context.expression = compiler_expression();

}
void tokenize_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& ctx)
{
	if (it == end)
		return;

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false) {
		ctx.stack.assign_to_stack_if_possible(it);

		if (ctx.stack.time_to_exit())
			return;
	}

	const auto token = it->get();

	if(token->is_punctuation() == false)
		throw compile_error(token, "expected an operator or ';' instead of %s", token->string.c_str());

	auto op = dynamic_cast<punctuation_token_t*>(token);

	if (!satisfies_operator(op->punc)) {
		throw compile_error(token, "expected an operator or ';' instead of %s", token->string.c_str());

	}

	if(op->punc == punctuation_e::P_ASSIGN)
		throw compile_error(token, "don't use assignment operators in expressions");

	compiler_expression expression;
	expression.op = true;
	expression.identifier = token;
	ctx.expressions.push_back(expression);
	std::advance(it, 1);
	return;

}
[[nodiscard]] VectorTokenPtr::iterator evaluate_expression_sanity(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, const expression_token_stack& stack)
{
	if (it == end)
		throw compile_error(it->get(), "an empty expression is not allowed");

	expression_context ctx;
	ctx.stack = stack;

	while (it != end) {
		tokenize_operand(it, end, ctx);

		if (ctx.stack.time_to_exit()) {
			it = ctx.stack.location;
			LOG("returning to " << it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");
			break;
		}
		if (it->get()->is_operator(punctuation_e::P_SEMICOLON)) {
			break;
		}

		tokenize_operator(it, end, ctx);

	}

	if (ctx.expressions.empty()) {
		throw compile_error(it->get(), "an empty expression is not allowed");
	}

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false)
		throw compile_error(it->get(), "expected to find a '%s'\n", punctuations[ctx.stack.closing].identifier.c_str());

	LOG("there are " << ctx.expressions.size() << " parts to this expression\n");
	
	return it;
}

bool peek_unary_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context)
{
	if (it == end)
		return false;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return false;

	if (it->get()->is_punctuation() == false)
		return false;
	
	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());

	if (bad_unary_operator(ptr->punc))
		throw compile_error("'%s' is not a valid unary operator", ptr->string.c_str());

	if (!is_unary_operator(ptr->punc)) {
		return false;
	}

	context.expression.prefix.push_back(it->get());
	std::advance(it, 1);
	return true;
}
bool peek_identifier(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context)
{
	if (it == end)
		return false;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return true;

	if (it->get()->is_punctuation())
		return false;
	
	auto& expression = context.expression;

	expression.identifier = it->get();

	//test if it's an identifier
	if (expression.identifier->tt == tokentype_t::IDENTIFIER) {

		//make sure it exists in the stack before it's used
		auto scope = compiler_data::getInstance().active_scope;

		if (scope->variable_exists(expression.identifier->string) == false) {
			throw compile_error(expression.identifier, "the identifier '%s' is undefined", expression.identifier->string.c_str());
		}
	}
	else if (expression.identifier->is_reserved_keyword())
		throw compile_error(expression.identifier, "the '%s' keyword is a reserved keyword and it cannot be used here", expression.identifier->string.c_str());


	std::advance(it, 1);
	return true;
}
bool peek_postfix_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, expression_context& context)
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

	if (ptr->punc == P_BRACKET_OPEN || ptr->punc == P_PAR_OPEN) {
		//check the next token
		if (VECTOR_PEEK(it, 1, end) == false) {
			throw compile_error("no way\n");
		}

		//if (context.stack.stack.num_open > 1)
		//	throw compile_error(it->get(), "this shouldn't happen");

		std::advance(it, 1);

		auto closing = static_cast<punctuation_e>(ptr->punc + 1);
		auto stack = expression_token_stack(ptr->punc, closing);

		LOG("jumping from " << it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");

		it = evaluate_expression_sanity(it, end, stack);

		if (it->get()->is_operator(closing) == false) {
			throw compile_error(it->get(), "expected a '%s'", punctuations[closing].identifier.c_str());
		}

		context.stack.stack.num_close++;
		++it;
		context.stack.location = it;
		



	}
	else
		std::advance(it, 1);


	return true;
}