#include "pch.hpp"

#include "linting_scope.hpp"
#include "linting_exceptions.hpp"
#include "linting_evaluate.hpp"

linting_scope* linting_create_scope_without_range(linting_scope* block)
{
	LOG("creating a scope\n");

	linting_scope* scope = (new linting_scope); //one could say this is bad but it's so well memory managed that it's ok :)
	
	scope->is_inside_of_a_function = block->is_inside_of_a_function;
	scope->returning_allowed = block->returning_allowed;
	scope->lower_scope = block;

	return scope;
}
linting_scope* linting_create_scope_for_function(linting_scope* block, function_def* def, bool returning_allowed)
{
	block = linting_create_scope_without_range(block);
	block->is_inside_of_a_function = true;
	block->returning_allowed = returning_allowed;

	for (auto& var : def->variables) {
		block->declare_variable(var);
	}

	return block;
}
linting_scope* linting_delete_scope([[maybe_unused]] ListTokenPtr::iterator& it, linting_scope* block)
{
	LOG("deleting the scope\n");

	if (block->is_global_scope()) {
		throw linting_error(it->get(), "found a '}' but it's not closing anything\n");
	}

	linting_scope* temp_block = block->lower_scope;

	auto& f = linting_data::getInstance().current_function;
	
	if (f && f->nest_depth)
		--f->nest_depth;

	//a function ends here so push it to the table
	//if (block->is_inside_of_a_function && block->lower_scope->is_inside_of_a_function == false) {
	//	linting_data::getInstance().function_declare(f); //note: std::move was called on f so it is no longer valid
	//}

	delete block;
	block = nullptr;

	return temp_block;
}