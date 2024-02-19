#include "pch.hpp"

#include "linting_evaluate.hpp"

[[nodiscard]] ListTokenPtr::iterator evaluate_subscript_sanity(
	ListTokenPtr::iterator it, 
	[[maybe_unused]]ListTokenPtr::iterator& end, 
	[[maybe_unused]] expression_context& context,
	operatorlist::iterator& pos)
{
	//throw linting_error(it->get(), "don't do subscript yet!!!");

	//check the next token
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("no way\n");
	}

	std::advance(it, 1);

	if (it->get()->is_operator(P_BRACKET_CLOSE) == true) { // no contents, not allowed
		throw linting_error(it->get(), "expected an expression instead of ']'");
	}

	auto block = std::make_unique<expression_block>();
	expression_token_stack stack(P_BRACKET_OPEN, P_BRACKET_CLOSE);
	stack.item_list = false;
	l_expression_results results = evaluate_expression_sanity(it, end, block, stack);

	it = results.it;

	if (context.stack.in_use() && context.stack.opening == P_BRACKET_OPEN) {
		context.stack.stack.num_close++;
		context.stack.location = it;
	}

	pos->get()->block = std::move(block);

	return it;

}

[[nodiscard]] ListTokenPtr::iterator evaluate_function_call_sanity(
	ListTokenPtr::iterator it, 
	ListTokenPtr::iterator& end, 
	[[maybe_unused]] expression_context& context,
	operatorlist::iterator& pos
	)
{
	//check the next token
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("no way\n");
	}

	LOG("calling evaluate_function_call_sanity()\n");

	std::advance(it, 1);

	

	auto call_block = std::make_unique<function_call>();
	call_block->target = linting_data::getInstance().get_function(context.identifier)->second.get();

	//parse the arguments
	if (it->get()->is_operator(P_PAR_CLOSE) == true) { // a function call with 0 arguments, return immediately 
		context.operators.back()->block = std::move(call_block);
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
	if (context.stack.in_use() && context.stack.opening == P_PAR_OPEN) {
		context.stack.stack.num_close++;
		context.stack.location = it;
	}
	
	pos->get()->block = std::move(call_block);

	return it;
}
