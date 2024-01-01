#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"


void evaluate_if_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, const std::unique_ptr<conditional_block>& chain)
{
	auto& data = linting_data::getInstance();

	if (data.active_scope->is_function_scope() == false)
		throw linting_error(it->get(), "an if statement is only allowed within function scopes");

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '(' instead of EOF");
	}
	else if (std::next(it)->get()->is_operator(P_PAR_OPEN) == false) {
		throw linting_error(std::next(it)->get(), "expected a '('");
	}

	std::advance(it, 2); //skip the if keyword and the '('

	if (it == end)
		throw linting_error(std::prev(it)->get(), "why does the file end here");


	if (chain == nullptr) {
		auto block = std::make_unique<conditional_block>(); //create data for runtime
		block->condition = std::make_unique<expression_block>();
		auto if_block = move_block_to_current_context(block);

		expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
		it = evaluate_expression_sanity(it, end, if_block->condition, stack).it;
	}
	else { //this means that this is an else if statement and chain points to the next block
		chain->condition = std::make_unique<expression_block>();
		expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
		it = evaluate_expression_sanity(it, end, chain->condition, stack).it;
	}

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->scope_type = scope_type_e::IF;

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

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("didn't expect the file to end here");
	}

	data.active_scope->emit_to_lower_scope(scope_type_e::IF);

}
void evaluate_else_sanity([[maybe_unused]]ListTokenPtr::iterator& it, [[maybe_unused]] ListTokenPtr::iterator& end)
{
	auto& data = linting_data::getInstance();

	auto ctx = data.active_scope->get_previous_scope_context();

	if (ctx.has_value() == false || ctx.value() != scope_type_e::IF)
		throw linting_error(it->get(), "the 'else' keyword is only allowed after an 'if' block");

	if(VECTOR_PEEK(it, 1, end) == false){
		throw linting_error(it->get(), "expected a '{'");
	}


	std::advance(it, 1);
	
	//get the if statement
	auto block = dynamic_cast<conditional_block*>(data.current_function->blocks.back());
	
	//go to the deepest nest
	while (block->next) {
		block = block->next.get();
	}
	//prepare the next block
	block->next = std::make_unique<conditional_block>();

	//else if block
	if (it->get()->tt == tokentype_t::IF) {
		evaluate_if_sanity(it, end, block->next);
		return;
	}

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(it->get(), "expected a '{'");
	}

	if(VECTOR_PEEK(it, 1, end) == false)
		throw linting_error(it->get(), "unexpected eof");

	if (std::next(it)->get()->is_operator(P_CURLYBRACKET_CLOSE)) {
		throw linting_error(std::next(it)->get(), "empty block");
	}

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->scope_type = scope_type_e::ELSE;
	data.active_scope->emit_to_lower_scope(scope_type_e::ELSE);

}
