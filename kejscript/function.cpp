#include "function.hpp"
#include "runtime.hpp"
#include "runtime_expression.hpp"

//these pointers don't get memory managed here
std::unique_ptr<operand> call_function(
	function_block* caller, 
	function_block* callee, 
	const std::list<std::unique_ptr<operand>>& args, 
	[[maybe_unused]]function_stack* stack, 
	struct function_stack** main_stack)
{
	LOG("caller: " << caller << '\n');


	auto unique_stack = std::make_unique<function_stack>(); //no need to manually reset this
	auto stack_ptr = unique_stack.get();
	auto arg = args.begin();
	for (const auto& param : callee->def.parameters) {
		stack_ptr->variables.push_back(std::make_shared<variable>(param));
		evaluation_functions::assign_to_lvalue(stack_ptr->variables.back().get(), *arg->get());
		++arg;
	}

	for (auto& v : callee->def.variables) {
		stack_ptr->variables.push_back(std::make_shared<variable>(v));
	}

	for (auto& instruction : callee->instructions) {

		if (instruction->execute(stack_ptr)) {
			auto returned_value = std::unique_ptr<operand>(callee->return_value); //claim ownership!

			if(!returned_value->is_object())
				returned_value->lvalue_to_rvalue();

			callee->return_value = 0;
			return returned_value;

		}
		
	}
	if (main_stack && caller->entrypoint) { //give the information back to the entrypoint
		*main_stack = unique_stack.release();
		return nullptr;
	}

	unique_stack.reset();
	return nullptr;
}