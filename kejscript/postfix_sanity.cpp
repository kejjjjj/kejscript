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
	auto& data = linting_data::getInstance();


	const auto convert_to_struct = [&context, &data](std::unique_ptr<function_call>& block) {
			
		auto struct_block = std::make_unique<struct_call>();

		struct_block->arguments = std::move(block->arguments);
		struct_block->target = std::move(block->target);
		struct_block->target_struct = data.get_struct_data(context.identifier);

		return std::move(struct_block);
		};

	//check the next token
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("no way\n");
	}

	struct_def* struct_block = 0;
	std::advance(it, 1);

	auto call_block = std::make_unique<function_call>();

	if (data.struct_exists(context.identifier)) {
		struct_block = data.get_struct_data(context.identifier);
	}
	else {
		call_block->target = data.get_function(context.identifier);
	}

	if (context.stack.stack.num_open)
		context.stack.stack.num_open = 1;

	if (context.stack.in_use() && context.stack.opening == P_PAR_OPEN) {
		context.stack.stack.num_close++;
		context.stack.location = it;
	}

	//parse the arguments
	if (it->get()->is_operator(P_PAR_CLOSE) == true) { // a function call with 0 arguments, return immediately 

		if (struct_block) {
			function_block* constructor_block = 0;

			for (auto& constructor : struct_block->constructors) {
				if (constructor->def.parameters.empty()) {
					constructor_block = constructor.get();
					break;
				}
			}
			call_block->target = constructor_block;
			context.operators.back()->block = std::move(convert_to_struct(call_block));
		}
		else
			context.operators.back()->block =  std::move(call_block);


		return it;
	}

	//std::cout << "from: " << it->get()->string << ": " << it->get()->format_position() << '\n';

	expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
	call_block->arguments = std::make_unique<expression_block>();
	l_expression_results results = evaluate_expression_sanity(it, end, call_block->arguments, stack);

	size_t params = 0;
	auto start = call_block->arguments.get();
	
	while (start) {
		++params;
		start = start->next.get();
	}

	if (struct_block) {
		function_block* constructor_block = 0;

		for (auto& constructor : struct_block->constructors) {

			if (constructor->def.parameters.size() == params) {
				constructor_block = constructor.get();
				break;
			}

		}

		if (!constructor_block) {
			std::string as_str = std::to_string(params);
			throw linting_error(it->get(), "no constructor of '%s' takes '%s' arguments", 
				context.identifier.c_str(), as_str.c_str());
		}
		
		call_block->target = constructor_block;

	}

	it = results.it;

	//std::cout << context.stack.in_use() << '\n';

	//if (!context.stack.in_use())
	//	std::advance(it, 1);

	//std::cout << "to: " << it->get()->string << ": " << it->get()->format_position() << '\n';

	if (context.stack.in_use() && context.stack.opening == P_PAR_OPEN)
		context.stack.location = ++it;

	if (struct_block)
		pos->get()->block = std::move(convert_to_struct(call_block));
	else
		pos->get()->block = std::move(call_block);

	return it;
}

[[nodiscard]] ListTokenPtr::iterator  evaluate_member_access_sanity(
	ListTokenPtr::iterator it,
	ListTokenPtr::iterator& end,
	operatorlist::iterator& pos
)
{

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("no way\n");
	}

	std::advance(it, 1); 


	if (it->get()->is_reserved_keyword())
		throw linting_error(it->get(), "the '%s' keyword is a reserved keyword and it cannot be used here", it->get()->string.c_str());


	if (it->get()->tt != tokentype_t::IDENTIFIER) {
		throw linting_error(it->get(), "expected an identifier");
	}

	auto block = std::make_unique<member_access_block>();

	block->member = it->get()->string;
	pos->get()->block = std::move(block);

	//std::cout << it->get()->string << '\n';

	return it;
}