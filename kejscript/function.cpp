#include "function.hpp"
#include "runtime.hpp"
#include "runtime_expression.hpp"

//these pointers don't get memory managed here
std::unique_ptr<operand> call_function(function_block* caller, function_block* callee, expression_block* args)
{
	callee->caller = caller;
	auto stack = std::make_unique<function_stack>();

	for (auto& param : callee->def.parameters) {

		stack->variables.push_back(std::make_unique<variable>(param));
		stack->variables.back()->value = std::move(evaluate_expression(caller, args->ast_tree)->get_value_move());
		stack->variables.back()->initialized = true;
	}
	for (auto& v : callee->def.variables) {
		stack->variables.push_back(std::make_unique<variable>(v));
	}

	callee->stack = stack.get();
	for (auto& instruction : callee->instructions) {

		instruction->execute();

		if (instruction->type() == code_block_e::RETURN) {
			
			
			

			auto returned_value = std::unique_ptr<operand>(callee->return_value); //claim ownership!
			int value = *reinterpret_cast<int*>(returned_value->get_value()->value.data());

			stack.reset();
			return 	create_rvalue<integer_dt>(integer_dt(value));
		}
	}

	LOG("\n\n-----returning from: " << callee->def.identifier << "-----\n\n");
	for (auto& v : stack->variables) {
		if (v->value)
			LOG(std::format("{}<{}> = {}\n", v->identifier, v->value->type_str(), v->value->value_str()));
		else
			LOG(std::format("{}<uninitialized>\n", v->identifier));
	}

	stack.reset();
	return nullptr;
}