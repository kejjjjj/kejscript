#include "function.hpp"
#include "runtime.hpp"

//one day this will return something
void call_function(std::unique_ptr<function_block>& target)
{
	//this will crash the program if you return from a function!
	runtime::stack.reset();
	runtime::stack = std::make_unique<function_stack>();

	//this shit is fucking stupid
	//the stack shouldn't be a global!!!
	for (auto& var : target->def.variables)
		runtime::stack->variables.push_back(std::make_unique<variable>(var));

	for (auto& instruction : target->instructions)
	{
		instruction->execute();
	}

}