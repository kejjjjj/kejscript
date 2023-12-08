#include "runtime_declaration.hpp"
#include "runtime.hpp"

void declare_variable(VectorTokenPtr::iterator& it)
{
	std::advance(it, 1); //skip the def keyword

	runtime::get_instance().declare_variable(it->get()->string);



}