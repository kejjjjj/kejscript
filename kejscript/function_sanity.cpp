#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"
#include "function_sanity.hpp"

void unevaluated_function::evaluate(ListTokenPtr::iterator& end, bool returning_allowed) const {

	auto& data = linting_data::getInstance();
	data.current_function = target;
	data.active_scope = linting_create_scope_for_function(data.active_scope, &target->def, returning_allowed);
	auto it = block_start;

	while (data.active_scope->is_function_scope())
	{
		evaluate_identifier_sanity(it, end);
		++it;
	}


}
ListTokenPtr::iterator skip_function(ListTokenPtr::iterator it, ListTokenPtr::iterator& end)
{
	if (it == end)
		throw linting_error(std::prev(it)->get(), "unexpected EOF");


	if (!it->get()->is_operator(P_CURLYBRACKET_OPEN)) {
		throw linting_error(it->get(), "expected '{' (internal bug)");
	}

	return seek_block_end(it, end);

}

void parse_parameters(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, function_def& def);

std::unique_ptr<function_block> parse_function_declaration(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	function_def funcdef;
	funcdef.identifier = it->get()->string;

	//make sure that next token is (
	if (VECTOR_PEEK(it, 1, end) == false || std::next(it)->get()->is_operator(P_PAR_OPEN) == false) {
		throw linting_error(it->get(), "expected a '('");
	}

	std::advance(it, 1); //skip the (

	//start parsing the parameters

	if (VECTOR_PEEK(it, 1, end) == false)
		throw linting_error(it->get(), "WHAT ARE THESE RANDOM END OF FILES!!!!");

	//if the the next punctuation mark is a closing parenthesis then there is no point in parsing the parameters
	if (std::next(it)->get()->is_operator(P_PAR_CLOSE) == false) {
		parse_parameters(it, end, funcdef);
	}
	else
		std::advance(it, 1); //skip to the ')' if there are no parameters

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' but encountered EOF");
	}
	else if (std::next(it)->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(std::next(it)->get(), "expected a '{'");
	}

	std::advance(it, 1); //because the scope was created in this function, skip the { token to avoid double scope creation

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '}' but encountered EOF");
	}

	auto func = std::make_unique<function_block>(funcdef);

	return std::move(func);
}

void evaluate_function_declaration_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	auto& data = linting_data::getInstance();
	
	if (data.active_scope->is_global_scope() == false) {
		throw linting_error(it->get(), "function declarations are only allowed in the global scope");
	}

	if (VECTOR_PEEK(it, 1, end) == false || std::next(it)->get()->is_identifier() == false) {
		throw linting_error(it->get(), "expected a name for the function");
	}

	std::advance(it, 1); //skip fn

	auto func = parse_function_declaration(it, end);


	//create the scope for the function
	
	//data.active_scope = linting_create_scope_for_function(data.active_scope, &func->def, true);

	if (func->def.identifier == "main")
		func->entrypoint = true;

	data.current_function = data.function_declare(func).get();
	data.unevaluated_functions.insert({ data.current_function->def.identifier , { data.current_function, std::next(it) } });

	it = skip_function(it, end);
}

void parse_parameters(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, function_def& def)
{
	
	if (it == end)
		throw linting_error(it->get(), "expected to find a ')'");

	//assumes that "it" contains a , or a ( right now

	//read the identifier
	if (VECTOR_PEEK(it, 1, end) == false) { 
		throw linting_error(it->get(), "expected an identifier");
	}
	else if (std::next(it)->get()->is_identifier() == false) {
		throw linting_error(std::next(it)->get(), "expected an identifier");
	}

	std::advance(it, 1);
	def.parameters.push_back(it->get()->string);
	def.operands.insert({ it->get()->string, def.operands.size() });

	for (auto& var : def.variables) {

		if(var == it->get()->string)
			throw linting_error(it->get(), "the parameter '%s' has already been declared", it->get()->string.c_str());

	}

	def.variables.push_back(it->get()->string);

	if (VECTOR_PEEK(it, 1, end) == false)  //ridiculous amount of eof checks...
		throw linting_error(it->get(), "expected to find a ')'");
	
	std::advance(it, 1);

	if (it->get()->is_operator(punctuation_e::P_COMMA)) 
		return parse_parameters(it, end, def); //go to next parameter
	
	if (it->get()->is_operator(punctuation_e::P_PAR_CLOSE) == false)
		throw linting_error(it->get(), "expected to find a ')'");

	//last parameter
	return;

}
void evaluate_return_sanity(ListTokenPtr::iterator& it, [[maybe_unused]]ListTokenPtr::iterator& end)
{
	//in case I forget :blush:
	auto& c_data = linting_data::getInstance();

	if (c_data.active_scope->is_function_scope() == false)
		throw linting_error(it->get(), "the 'return' keyword is only allowed within functions");

	//check if the next token is valid
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected an expression before EOF");
	}

	//skip the return keyword
	std::advance(it, 1);

	auto ret = std::make_unique<return_statement>();

	if (it->get()->is_operator(punctuation_e::P_SEMICOLON)) { //no point in parsing the expression after if there is no expression
		move_block_to_current_context(ret);
		return;
	}

	if (c_data.active_scope->returning_allowed == false)
		throw linting_error(it->get(), "returning a value from this function is not allowed");

	ret->expression = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, end, ret->expression).it;
	move_block_to_current_context(ret);
	
}