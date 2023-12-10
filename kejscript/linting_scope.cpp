#include "pch.hpp"

#include "linting_scope.hpp"
#include "linting_exceptions.hpp"
#include "linting_evaluate.hpp"
linting_scope* linting_create_scope_without_range(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, linting_scope* block)
{
	LOG("creating a scope\n");

	linting_scope* scope = (new linting_scope); //one could say this is bad but it's so well memory managed that it's ok :)
	
	if (block->is_inside_of_a_function)
		scope->is_inside_of_a_function = true;

	scope->lower_scope = block;

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN))
		linting_data::getInstance().remove_token(it, end);

	return scope;
}
linting_scope* linting_delete_scope([[maybe_unused]] ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, token_t* token, linting_scope* block)
{
	LOG("deleting the scope\n");

	if (block->is_global_scope()) {
		throw linting_error(token, "found a '}' but it's not closing anything\n");
	}

	block->print_stack();
	linting_scope* temp_block = block->lower_scope;

	if (block->is_inside_of_a_function && block->lower_scope->is_inside_of_a_function == false) {
		auto& f = linting_data::getInstance().current_function;
		f.end = std::prev(it);
		LOG(std::format("creating a func from [{}, {}] to [{}, {}]\n", f.start->get()->line, f.start->get()->column, f.end->get()->line, f.end->get()->column));

		linting_data::getInstance().function_declare(std::move(f));
		f = function_def();
	}
	
	if (it->get()->is_operator(P_CURLYBRACKET_CLOSE))
		linting_data::getInstance().remove_token(it, end);

	if (block->block) {
		block->block->end = it;

		if (VECTOR_PEEK(it, 1, end) == false)
			throw linting_error(it->get(), "weird eof");

		//peek if next token is 'else' and if it isn't then this block chain can end
		if (std::next(it)->get()->tt != tokentype_t::ELSE) {
			LOG("clearing if blocks\n");
			temp_block->block = nullptr;
		}
		else {
			temp_block->block = block->block;
		}
	}

	block = nullptr;



	return temp_block;
}