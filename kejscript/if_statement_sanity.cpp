#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

void evaluate_if_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	auto& data = linting_data::getInstance();

	if (data.active_scope->is_function_scope() == false)
		throw linting_error(it->get(), "an if statement is only allowed within function scopes");

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '(' instead of EOF");
	}
	else if (it[1].get()->is_operator(P_PAR_OPEN) == false) {
		throw linting_error(it[1].get(), "expected a '('");
	}

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	
	std::advance(it, 2); //skip the if keyword and the '('

	expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
	it = evaluate_expression_sanity(it, end, stack).it;

	if (it->get()->is_operator(P_PAR_CLOSE) == false) {
		throw linting_error(it->get(), "expected a '%s'", punctuations[P_PAR_CLOSE].identifier.c_str());
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' instead of EOF");
	}

	std::advance(it, 1); //skip the ')'

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(it->get(), "expected a '{'");

	}

	data.active_scope->emit_to_lower_scope(tokentype_t::IF);

}
void evaluate_else_sanity([[maybe_unused]]VectorTokenPtr::iterator& it, [[maybe_unused]] VectorTokenPtr::iterator& end)
{
	auto& data = linting_data::getInstance();

	auto ctx = data.active_scope->get_previous_scope_context();

	if (ctx.has_value() == false || ctx.value() != tokentype_t::IF)
		throw linting_error(it->get(), "the 'else' keyword is only allowed after an 'if' block");

	if(VECTOR_PEEK(it, 1, end) == false){
		throw linting_error(it->get(), "expected a '{'");
	}

	std::advance(it, 1);

	if (it->get()->tt == tokentype_t::IF) {
		return evaluate_if_sanity(it, end);
	}

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(it->get(), "expected a '{'");
	}

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->emit_to_lower_scope(tokentype_t::ELSE);

}
