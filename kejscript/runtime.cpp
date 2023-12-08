#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"

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

	while (it != end) {

		it = evaluate_expression(it, end)->it;

		++it;
	}

}