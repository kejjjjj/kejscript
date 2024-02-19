#include "pch.hpp"
#include "linting_evaluate.hpp"

static bool initialization_has_declaration(ListTokenPtr::iterator& it)
{
	return it->get()->tt == tokentype_t::DEF;
}
[[nodiscard]] static auto declare_variable(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, linting_data& data)
{
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected an expression instead of EOF");
	}

	if (it->get()->is_identifier() == false)
		throw linting_error(it->get(), "expected an identifier");

	if (data.active_scope->declare_variable(it->get()->string) == false)
		throw linting_error(it->get(), "the variable '%s' is already defined");

	LOG("declaring: '" << it->get()->string << "'\n");

	auto& funcdef = data.current_function->def;
	funcdef.variables.push_back(it->get()->string);

	//check if the next token is valid
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error(it->get(), "the variable must be initialized");
	}

	if (std::next(it)->get()->is_operator(P_ASSIGN) == false) {
		throw linting_error(it->get(), "the variable must be initialized with '='");
	}
	
	auto block = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, end, block).it;

	return std::move(block);
}
[[nodiscard]] static auto parse_expression(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, const expression_token_stack& stack = expression_token_stack())
{
	auto block = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, end, block, stack).it;

	return std::move(block);
}
void evaluate_for_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	auto& data = linting_data::getInstance();

	if (data.active_scope->is_function_scope() == false)
		throw linting_error(it->get(), "a for statement is only allowed within function scopes");

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '(' instead of EOF");
	}

	else if (std::next(it)->get()->is_operator(P_PAR_OPEN) == false) {
		throw linting_error(std::next(it)->get(), "expected a '('");
	}

	auto block = std::make_unique<for_block>(); //create data for runtime
	//block->condition = std::make_unique<expression_block>();

	auto statement_block = move_block_to_current_context(block);

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->scope_type = scope_type_e::WHILE;

	std::advance(it, 1); //skip the for keyword
	std::advance(it, 1); //skip the '('

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error(it->get(), "expected an expression");
	}

	//initialization field

	//next keyword is def
	if (initialization_has_declaration(it)) {
		std::advance(it, 1); //skip the def keyword
		statement_block->initialization = std::move(declare_variable(it, end, data));
	}

	else if(it->get()->is_operator(P_SEMICOLON) == false) {
		statement_block->initialization = std::move(parse_expression(it, end));
	}

	if (it->get()->is_operator(P_SEMICOLON) == false) 
		throw linting_error(it->get(), "expected a ';'");

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error(it->get(), "expected an expression");
	}

	std::advance(it, 1);

	//condition field
	
	//has condition
	if (it->get()->is_operator(P_SEMICOLON) == false) {
		statement_block->condition = std::move(parse_expression(it, end));

		if (it->get()->is_operator(P_SEMICOLON) == false)
			throw linting_error(it->get(), "expected a ';'");
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error(it->get(), "expected an expression");
	}
	
	std::advance(it, 1);

	//execution field
	if (it->get()->is_operator(P_PAR_CLOSE) == false) {
		statement_block->execution = std::move(parse_expression(it, end, { P_PAR_OPEN, P_PAR_CLOSE }));

		if (it->get()->is_operator(P_PAR_CLOSE) == false)
			throw linting_error(it->get(), "expected a ')'");
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' instead of EOF");
	}

	std::advance(it, 1); //skip the )

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(it->get(), "expected a '{'");
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("didn't expect the file to end here");
	}

	
}