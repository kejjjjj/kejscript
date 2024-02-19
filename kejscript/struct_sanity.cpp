#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

static std::unique_ptr<variable_initializer> evaluate_struct_variable_declaration(linting_data& data, 
	ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	auto scope = data.active_scope;

	std::advance(it, 1); //skip the def keyword

	if(it == end)
		throw linting_error("expected an identifier instead of EOF");

	if (it->get()->is_identifier() == false)
		throw linting_error(it->get(), "expected an identifier");

	if (scope->declare_variable(it->get()->string) == false)
		throw linting_error(it->get(), "the variable '%s' is already defined", it->get()->string.c_str());

	LOG("declaring: '" << it->get()->string << "'\n");

	std::unique_ptr<variable_initializer> init = std::make_unique<variable_initializer>();

	init->variable = it->get()->string;

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error(it->get(), "expected a '=' or ';'");
	}

	std::advance(it, 1);

	if (it->get()->is_operator(P_SEMICOLON)) {
		return std::move(init);
	}

	if (it->get()->is_punctuation()) {
		const auto punc = dynamic_cast<punctuation_token_t*>(it->get())->punc;

		if (punc != punctuation_e::P_ASSIGN && punc != punctuation_e::P_SEMICOLON)
			throw linting_error(it->get(), "expected a '=' or ';'");

		std::advance(it, 1); //skip the =

		if (it == end)
			throw linting_error("expected an expression instead of EOF");
	}

	init->initializer = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, end, init->initializer).it;
	return std::move(init);

}

void evaluate_struct_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected an identifier instead of EOF");
	}

	std::advance(it, 1); //skip the struct keyword

	auto& data = linting_data::getInstance();
	std::unique_ptr<struct_def> def = std::make_unique<struct_def>();

	if (it->get()->is_identifier() == false) {
		throw linting_error(std::next(it)->get(), "expected an identifier");
	}

	if (data.struct_exists(it->get()->string)) {
		throw linting_error(it->get(), "the struct '%s' is already defined");
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' instead of EOF");
	}

	def->identifier = it->get()->string;

	std::advance(it, 1); //skip the identifier keyword

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(std::next(it)->get(), "expected a '{'");
	}

	std::advance(it, 1); //skip the the '{'

	if (it == end)
		throw linting_error(std::prev(it)->get(), "why does the file end here");

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->scope_type = scope_type_e::STRUCT;

	while (it != end && it->get()->is_operator(P_CURLYBRACKET_CLOSE) == false) {

		switch (it->get()->tt) {
		case tokentype_t::DEF:
			def->initializers.push_back(std::move(evaluate_struct_variable_declaration(data, it, end)));
			break;
		default:
			throw linting_error(it->get(), "expected a variable declaration");
		}

		std::advance(it, 1);

	}

	if (it == end)
		throw linting_error(std::prev(it)->get(), "why does the file end here");

	data.struct_declare(def);
	
	std::advance(it, -1);//go back to the token before the }

	//throw linting_error("gg");

}