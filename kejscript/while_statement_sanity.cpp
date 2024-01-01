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
	
	auto block = std::make_unique<while_block>(); //create data for runtime
	block->condition = std::make_unique<expression_block>();

	auto while_block = move_block_to_current_context(block);

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->scope_type = scope_type_e::WHILE;


	std::advance(it, 1); //skip the while keyword
	std::advance(it, 1); //skip the '('


	if (it == end)
		throw linting_error(std::prev(it)->get(), "why does the file end here");

	expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
	it = evaluate_expression_sanity(it, end, while_block->condition, stack).it;

	if (it->get()->is_operator(P_PAR_CLOSE) == false) {
		throw linting_error(it->get(), "expected a '%s'", punctuations[P_PAR_CLOSE].identifier.c_str());
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' instead of EOF");
	}

	std::advance(it, 1);

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(it->get(), "expected a '{'");
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("didn't expect the file to end here");
	}

	data.active_scope->emit_to_lower_scope(scope_type_e::WHILE);

}