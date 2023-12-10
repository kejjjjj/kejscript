#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"
#include "runtime_declaration.hpp"
#include "function.hpp"
#include "if_statement.hpp"
#include "while_statement.hpp"

void runtime::initialize(const ListTokenPtr::iterator begin, const ListTokenPtr::iterator end, const std::unordered_map<std::string, function_def>& table)
{
	_begin = begin;
	_end = end;
	function_table = table;


}
void runtime::execute()
{
	decltype(_begin) it = _begin;
	decltype(_end) end = _end;

	evaluation_functions::getInstance().initialize_functions();

	entry_point = &function_table.find("main")->second;

	transfer_code_execution(it, end, end, *entry_point);

	while (it != end) {

		while (stack->is_conditional_block()) {
			evaluate_token(stack->conditional_block->start, stack->conditional_block->end);

			if (stack->conditional_block->blockType == tokentype_t::IF) {
				if (handle_if_conditional(it, end, stack->conditional_block))
					break;
			}
			if (stack->conditional_block->blockType == tokentype_t::WHILE) {
				if (handle_while_conditional(it, stack->conditional_block))
					break;

			}

			
		}

		if (it == end)
			break;

		evaluate_token(it, end);

		if (it == end)
			break;

		++it;
	}
	
}
void evaluate_token(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	switch (it->get()->tt) {
	case tokentype_t::DEF:
		return declare_variable(it, end);
	case tokentype_t::IF:
		return evaluate_if_statement(it, end, dynamic_cast<if_block*>(it->get()->block.get()));
	case tokentype_t::WHILE:
		return evaluate_while_statement(it, end);
	}

	it = evaluate_expression(it, end)->it;

}