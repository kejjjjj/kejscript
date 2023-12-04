#include "pch.hpp"

compiler_scope* compiler_create_scope_without_range(compiler_scope* block)
{
	LOG("creating a scope\n");

	compiler_scope* scope = (new compiler_scope); //one could say this is bad but it's so well memory managed that it's ok :)
	
	scope->lower_scope = block;

	return scope;
}
compiler_scope* compiler_delete_scope(script_t& script, compiler_scope* block)
{
	LOG("deleting the scope\n");

	if (block->is_global_scope()) {
		throw compile_error(script.get_code_pos()->get(), "found a '}' but it's not closing anything\n");
	}

	block->print_stack();

	compiler_scope* temp_block = block->lower_scope;
	delete block;
	block = 0;

	return temp_block;
}