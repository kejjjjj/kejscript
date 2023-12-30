#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"
#include "runtime_declaration.hpp"
#include "function.hpp"
#include "if_statement.hpp"
#include "while_statement.hpp"
#include "runtime_exception.hpp"

void runtime::initialize(code_instruction& instrcs, const std::unordered_map<std::string, function_def>& table)
{
	instructions = std::move(instrcs);
	function_table = table;
}
void runtime::execute()
{
	//decltype(_begin) it = _begin;
	//decltype(_end) end = _end;


	evaluation_functions::getInstance().initialize_functions();

	entry_point = &function_table.find("main")->second;

	transfer_code_execution(end, *entry_point);

	while (it != end) {

		//while (stack->is_conditional_block()) {
		//	evaluate_token(stack->conditional_block->start, stack->conditional_block->end);

		//	if (stack->conditional_block->blockType == tokentype_t::IF) {
		//		if (handle_if_conditional(it, end, stack->conditional_block))
		//			break;
		//	}
		//	if (stack->conditional_block->blockType == tokentype_t::WHILE) {
		//		if (handle_while_conditional(it, stack->conditional_block))
		//			break;

		//	}

		//	
		//}

		if (it == end)
			break;

		evaluate_token(it, end);

		if (it == end)
			break;

		++it;
	}
	
}
void evaluate_token(ListTokenPtr::iterator& it, [[maybe_unused]] ListTokenPtr::iterator& end)
{
	switch (it->get()->tt) {
	case tokentype_t::DEF:
		throw runtime_error(it->get(), "STOP!");
	case tokentype_t::IF:
		throw runtime_error(it->get(), "STOP!");

		//return evaluate_if_statement(it, end, dynamic_cast<if_block*>(it->get()->block.get()));
	case tokentype_t::WHILE:
		throw runtime_error(it->get(), "STOP!");

		//return evaluate_while_statement(it, end);
	}

	const auto expression = dynamic_cast<expression_block*>(it->get()->block.get());
	evaluate_expression(expression->ast_tree);

}