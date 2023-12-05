#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

void evaluate_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	//check if the next token is valid
	if (VECTOR_PEEK(it, 1, end) == false) { 
		throw linting_error(it->get(), "expected an identifier");
	}
	std::advance(it, 1); // skip the init keyword

	if (it->get()->is_identifier() == false)
		throw linting_error(it->get(), "expected an identifier");

	auto scope = linting_data::getInstance().active_scope;

	if (!scope->declare_variable(it->get()->string))
		throw linting_error(it->get(), "the variable '%s' is already defined");

	LOG("declaring: '" << it->get()->string << "'\n");

	//check if the next token is valid
	if (VECTOR_PEEK(it, 1, end) == false || it[1].get()->is_operator(P_SEMICOLON)) {
		//if not, then there is no initializer
		std::advance(it, 1);
		return;
	}
	
	if (it[1].get()->is_punctuation()) {
		const auto punc = dynamic_cast<punctuation_token_t*>(it[1].get())->punc;

		if(punc != punctuation_e::P_ASSIGN && punc != punctuation_e::P_SEMICOLON)
			throw linting_error(it[1].get(), "expected a '=' or ';'");
	}
	
	std::advance(it, 2); //skip the identifier and the = punctuation

	it = evaluate_expression_sanity(it, end).it; //and now parse the expression 

}