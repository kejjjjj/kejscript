#include "function.hpp"
#include "runtime_declaration.hpp"
#include "runtime.hpp"

void transfer_code_execution(const ListTokenPtr::iterator& return_loc, const function_def& func)
{
	LOG("calling '" << func.identifier << "'\n");


	auto& rt = runtime::get_instance();

	rt.stack = std::make_unique<stack_t>(return_loc);

	for (const auto& decl : func.parameters)
		rt.stack->declare_variable(decl);

	for (const auto& decl : func.variables)
		rt.stack->declare_variable(decl);



}