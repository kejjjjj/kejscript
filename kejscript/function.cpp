#include "function.hpp"
#include "runtime.hpp"
#include "runtime_expression.hpp"

//these pointers don't get memory managed here
std::unique_ptr<operand> call_function(
	function_block* caller, 
	function_block* callee, 
	const std::list<operand*>& args, 
	[[maybe_unused]]function_stack* stack, 
	struct function_stack** main_stack)
{
	LOG("caller: " << caller << '\n');


	auto unique_stack = std::make_unique<function_stack>(); //no need to manually reset this
	auto stack_ptr = unique_stack.get();

	//create the stack
	size_t num_vars = callee->def.num_operands();

	for (size_t i = 0; i < num_vars; ++i) {
		stack_ptr->variables.push_back(std::make_shared<operand>());
		stack_ptr->variables[i]->value = std::make_shared<variable>();
	}

	auto arg = args.begin();
	size_t i = 0;
	for ([[maybe_unused]] const auto& _ : callee->def.parameters) {
		evaluation_functions::assign_to_lvalue(stack_ptr->variables[i]->get_lvalue(), *arg);
		++arg;
		++i;
	}

	//for (auto& v : callee->def.variables) {
	//	stack_ptr->variables.push_back(std::make_shared<variable>(v));
	//}

	for (auto& instruction : callee->instructions) {

		if (instruction->execute(stack_ptr)) {

			//auto& returned_value = stack_ptr->return_value; //claim ownership!

			//if (returned_value && !returned_value->valueless_type())
			//	returned_value->lvalue_to_rvalue();

			return nullptr;
			//return returned_value;

		}
		
	}
	if (main_stack && caller->entrypoint) { //give the information back to the entrypoint
		*main_stack = unique_stack.release();
		return nullptr;
	}

	unique_stack.reset();
	return nullptr;
}