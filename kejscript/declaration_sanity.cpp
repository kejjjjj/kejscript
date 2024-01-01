#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

void evaluate_declaration_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	auto scope = linting_data::getInstance().active_scope;

	if (scope->is_function_scope() == false)
		throw linting_error(it->get(), "variables can only be declared within function scopes");

	//check if the next token is valid
	if (VECTOR_PEEK(it, 1, end) == false) { 
		throw linting_error(it->get(), "expected an identifier");
	}
	//std::advance(it, 1); // skip the def keyword

	auto& data = linting_data::getInstance();

	std::advance(it, 1);

	if (it->get()->is_identifier() == false)
		throw linting_error(it->get(), "expected an identifier");

	if (scope->declare_variable(it->get()->string) == false)
		throw linting_error(it->get(), "the variable '%s' is already defined");

	LOG("declaring: '" << it->get()->string << "'\n");

	auto& funcdef = data.current_function->def;
	funcdef.variables.push_back(it->get()->string);

	//check if the next token is valid
	if (VECTOR_PEEK(it, 1, end) == false) {
		//if not, then there is no initializer
		std::advance(it, 1);
		return;
	
	}
	
	if (std::next(it)->get()->is_operator(P_SEMICOLON)) {
		std::advance(it, 1);
		return;
	}

	if (std::next(it)->get()->is_punctuation()) {
		const auto punc = dynamic_cast<punctuation_token_t*>(std::next(it)->get())->punc;

		if(punc != punctuation_e::P_ASSIGN && punc != punctuation_e::P_SEMICOLON)
			throw linting_error(it->get(), "expected a '=' or ';'");
	}
	
	auto block = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, end, block).it; //and now parse the expression 
	move_block_to_current_context(block);
}