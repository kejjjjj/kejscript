#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

void parse_parameters(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, linting_scope* scope, std::vector<std::string>& parameters);

void evaluate_function_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	auto& data = linting_data::getInstance();
	
	if (data.active_scope->is_global_scope() == false) {
		throw linting_error(it->get(), "function declarations are only allowed in the global scope");
	}

	if (VECTOR_PEEK(it, 1, end) == false || it[1].get()->is_identifier() == false) {
		throw linting_error(it->get(), "expected a name for the function");
	}


	std::advance(it, 1);
	
	data.current_function.identifier = it->get()->string;
	std::vector<std::string> parameters;

	//make sure that next token is (
	if (VECTOR_PEEK(it, 1, end) == false || it[1].get()->is_operator(P_PAR_OPEN) == false) {
		throw linting_error(it->get(), "expected a '('");
	}

	std::advance(it, 1);

	//create the scope for the function
	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->is_inside_of_a_function = true;
	//start parsing the parameters

	if(VECTOR_PEEK(it, 1, end) == false)
		throw linting_error(it->get(), "WHAT ARE THESE RANDOM END OF FILES!!!!");

	//if the the next punctuation mark is a closing parenthesis then there is no point in parsing the parameters
	if (it[1].get()->is_operator(P_PAR_CLOSE) == false) {
		parse_parameters(it, end, data.active_scope, data.current_function.parameters);
	}else
		std::advance(it, 1); //skip to the ')' if there are no parameters

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' but encountered EOF");
	}
	else if (it[1].get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(it[1].get(), "expected a '{'");
	}

	std::advance(it, 1); //because the scope was created in this function, skip the { token to avoid double scope creation

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '}' but encountered EOF");
	}

	data.current_function.start = (it + 1);

}

void parse_parameters(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, linting_scope* scope, std::vector<std::string>& parameters)
{
	
	if (it == end)
		throw linting_error(it->get(), "expected to find a ')'");

	//assumes that "it" contains a , or a ( right now

	//read the identifier
	if (VECTOR_PEEK(it, 1, end) == false) { 
		throw linting_error(it->get(), "expected an identifier");
	}
	else if (it[1].get()->is_identifier() == false) {
		throw linting_error(it[1].get(), "expected an identifier");
	}

	std::advance(it, 1);

	parameters.push_back(it->get()->string);

	if (!scope->declare_variable(parameters.back())) {
		throw linting_error(it->get(), "this parameter has already been declared");
	}

	if (VECTOR_PEEK(it, 1, end) == false)  //ridiculous amount of eof checks...
		throw linting_error(it->get(), "expected to find a ')'");
	
	std::advance(it, 1);

	if (it->get()->is_operator(punctuation_e::P_COMMA)) 
		return parse_parameters(it, end, scope, parameters); //go to next parameter
	
	if (it->get()->is_operator(punctuation_e::P_PAR_CLOSE) == false)
		throw linting_error(it->get(), "expected to find a ')'");

	//last parameter
	return;

}
void evaluate_return_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{

	auto& c_data = linting_data::getInstance();

	if (c_data.active_scope->is_inside_of_a_function == false)
		throw linting_error(it->get(), "the 'return' keyword is only allowed within function scopes");

	//check if the next token is valid
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected an expression before EOF");
	}

	//skip the return keyword
	std::advance(it, 1);

	if (it->get()->is_operator(punctuation_e::P_SEMICOLON) == false) { //no point in parsing the expression after if there is no expression
		it = evaluate_expression_sanity(it, end).it;
	}

	//LOG("done!\n");
	
}