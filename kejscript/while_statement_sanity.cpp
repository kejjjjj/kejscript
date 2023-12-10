#include "pch.hpp"
#include "linting_evaluate.hpp"

void evaluate_while_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{

	auto& data = linting_data::getInstance();

	if (data.active_scope->is_function_scope() == false)
		throw linting_error(it->get(), "a while statement is only allowed within function scopes");

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '(' instead of EOF");
	}

	else if (std::next(it)->get()->is_operator(P_PAR_OPEN) == false) {
		throw linting_error(std::next(it)->get(), "expected a '('");
	}

	data.active_scope = linting_create_scope_without_range(it, end, data.active_scope);
	data.active_scope->scope_type = tokentype_t::WHILE;
	auto target_token = it->get();


	auto block = std::make_unique<while_block>(it->get()); //create data for runtime

	std::advance(it, 2); //skip the while keyword and the '('

	block->condition_start = it;

	expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
	it = evaluate_expression_sanity(it, end, stack).it;

	if (it->get()->is_operator(P_PAR_CLOSE) == false) {
		throw linting_error(it->get(), "expected a '%s'", punctuations[P_PAR_CLOSE].identifier.c_str());
	}

	dynamic_cast<punctuation_token_t*>(it->get())->punc = P_SEMICOLON;
	block->condition_end = it;


	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' instead of EOF");
	}

	std::advance(it, 1); //skip the ')'

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(it->get(), "expected a '{'");
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("didn't expect the file to end here");
	}

	//remove the { token because it is no longer relevant
	data.remove_token(it, end);

	block->start = it;

	if (block->start->get()->is_operator(P_CURLYBRACKET_CLOSE)) {
		throw linting_error(block->start->get(), "empty block");
	}

	data.active_scope->emit_to_lower_scope(tokentype_t::WHILE);

	target_token->block = std::move(block);
	data.active_scope->block = target_token->block.get();


}