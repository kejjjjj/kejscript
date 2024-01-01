#include "pch.hpp"

#include "linting_evaluate.hpp"

[[nodiscard]] ListTokenPtr::iterator evaluate_subscript_sanity(
	ListTokenPtr::iterator it, 
	[[maybe_unused]]ListTokenPtr::iterator& end, 
	[[maybe_unused]] l_expression_context& context)
{
	throw linting_error(it->get(), "don't do subscript yet!!!");
	////check the next token
	//if (VECTOR_PEEK(it, 1, end) == false) {
	//	throw linting_error("no way\n");
	//}

	//std::advance(it, 1);

	//auto stack = expression_token_stack(P_BRACKET_OPEN, P_BRACKET_CLOSE);

	//LOG("jumping from " << it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");

	//it = evaluate_expression_sanity(it, end, stack).it;

	//if (it->get()->is_operator(P_BRACKET_CLOSE) == false) {
	//	throw linting_error(it->get(), "expected a '%s'", punctuations[P_BRACKET_CLOSE].identifier.c_str());
	//}

	//context.stack.stack.num_close++;
	//++it;
	//context.stack.location = it;

	//return it;

}

[[nodiscard]] ListTokenPtr::iterator evaluate_function_call_sanity(
	ListTokenPtr::iterator it, 
	ListTokenPtr::iterator& end, 
	[[maybe_unused]] l_expression_context& context, 
	singular* s, const std::string& target_func)
{
	//check the next token
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("no way\n");
	}

	LOG("calling evaluate_function_call_sanity()\n");

	std::advance(it, 1);

	s->callable = std::make_unique<function_call>();
	s->callable->target = linting_data::getInstance().get_function(target_func)->second.get();
	auto& call_block = s->callable;

	//parse the arguments
	if (it->get()->is_operator(P_PAR_CLOSE) == true) { // a function call with 0 arguments, return immediately 
		return it;
	}
	//get the list of the arguments
	expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
	LOG("jumping from " << it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");


	call_block->arguments = std::make_unique<expression_block>();
	l_expression_results results = evaluate_expression_sanity(it, end, call_block->arguments, stack);

	//size_t num_args = results.num_evaluations;
	it = results.it;

	

	//LOG("calling '" << context.expression.identifier->string << "' with " << num_args << " arguments!\n");

	context.stack.stack.num_close++;
	++it;
	context.stack.location = it;
	
	

	return it;
}
