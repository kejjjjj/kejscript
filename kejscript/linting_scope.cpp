#include "pch.hpp"

#include "linting_scope.hpp"
#include "linting_exceptions.hpp"

linting_scope* linting_create_scope_without_range(linting_scope* block)
{
	LOG("creating a scope\n");

	linting_scope* scope = (new linting_scope); //one could say this is bad but it's so well memory managed that it's ok :)
	
	if (block->is_inside_of_a_function)
		scope->is_inside_of_a_function = true;

	scope->lower_scope = block;

	return scope;
}
linting_scope* linting_delete_scope(token_t* token, linting_scope* block)
{
	LOG("deleting the scope\n");

	if (block->is_global_scope()) {
		throw linting_error(token, "found a '}' but it's not closing anything\n");
	}

	block->print_stack();

	linting_scope* temp_block = block->lower_scope;
	delete block;
	block = 0;

	return temp_block;
}