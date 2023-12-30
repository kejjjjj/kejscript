#include "runtime_declaration.hpp"
#include "runtime.hpp"
#include "runtime_expression.hpp"

void declare_variable(ListTokenPtr::iterator& it, [[maybe_unused]]ListTokenPtr::iterator& end)
{
	std::advance(it, 1); //skip the def keyword

	runtime::get_instance().stack->declare_variable(it->get()->string);
	//it = evaluate_expression(it, end)->it;

}