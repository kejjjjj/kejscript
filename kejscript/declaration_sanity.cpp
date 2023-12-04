#include "pch.hpp"

void evaluate_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	if (it->get()->tt != tokentype_t::IDENTIFIER)
		throw compile_error(it->get(), "expected an identifier");

	auto scope = compiler_data::getInstance().active_scope;

	scope->declare_variable(it->get()->string);
	LOG("declaring: '" << it->get()->string << "'\n");

	std::advance(it, 2); //skip the identifier and the = 

	evaluate_expression_sanity(it, end); //and now parse the expression 
}