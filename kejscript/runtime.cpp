#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"
#include "function.hpp"
#include "runtime_exception.hpp"

function_table_t runtime::function_table;
code_instruction runtime::instructions;

void runtime::execute()
{


	auto& entry_point = function_table.find("main")->second;


	std::chrono::time_point<std::chrono::steady_clock> old = std::chrono::steady_clock::now();

	auto main_stack = std::make_unique<function_stack>();

	for (auto& v : entry_point->def.variables) {
		main_stack->variables.push_back(std::make_unique<variable>(v));
	}
	auto ptr = main_stack.get();
	std::list<std::unique_ptr<operand>> a;
	call_function(entry_point.get(), entry_point.get(), a, main_stack.get(), &ptr);
	

	std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
	std::chrono::duration<float> difference = now - old;

	main_stack.reset();
	main_stack = std::unique_ptr<function_stack>(ptr);


	//LOG("\n\n-----returning from: " << entry_point->def.identifier << "-----\n\n");

	for (auto& v : main_stack->variables) {
		v->print();
	}
	std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";

	printf("\ntime taken: %.6f\n", difference.count());


}