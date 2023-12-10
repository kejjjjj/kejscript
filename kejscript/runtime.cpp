#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"
#include "runtime_declaration.hpp"
#include "function.hpp"

void runtime::initialize(const VectorTokenPtr::iterator begin, const VectorTokenPtr::iterator end, const std::unordered_map<std::string, function_def>& table)
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
		evaluate_token(it, end);

		if (it == end)
			break;

		++it;
	}

	stack->print_stack();

	
}
void evaluate_token(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	switch (it->get()->tt) {
	case tokentype_t::DEF:
		return declare_variable(it, end);
	}

	it = evaluate_expression(it, end)->it;

}