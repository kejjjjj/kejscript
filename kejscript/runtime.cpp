#include "pch.hpp"

#include "runtime.hpp"
#include "runtime_expression.hpp"
#include "operators.hpp"
#include "function.hpp"
#include "runtime_exception.hpp"

function_table_t runtime::function_table;
code_instruction runtime::instructions;
std::vector<function_block*> runtime::sorted_functions;
std::vector<script_literals*> runtime::sorted_literals;
std::vector<std::unique_ptr<operand>> runtime::literals;

void runtime::execute()
{
	literals = decltype(literals)(sorted_literals.size());

	for (size_t i = 0; i < sorted_literals.size(); i++) {

		auto& type = sorted_literals[i]->value;

		auto oper = std::make_unique<operand>();

		switch (type->rvalue_t) {

		case datatype_e::bool_t: 
			oper->make_rvalue(type->get<bool_dt>()); 
			break;
		case datatype_e::char_t:
			oper->make_rvalue(type->get<char_dt>());
			break;
		case datatype_e::int_t: 
			oper->make_rvalue(type->get<integer_dt>()); 
			break;
		case datatype_e::double_t:
			oper->make_rvalue(type->get<double_dt>());
			break;
		case datatype_e::string_t:
			oper->make_rvalue(type->get<string_dt>());
			break;
		}

		literals[i] = std::move(oper);

	}

	auto& entry_point = function_table.find("main")->second;


	std::chrono::time_point<std::chrono::steady_clock> old = std::chrono::steady_clock::now();

	auto main_stack = std::make_unique<function_stack>();

	auto stack_ptr = main_stack.get();

	//create the stack
	stack_ptr->variables = decltype(stack_ptr->variables)(entry_point->def.num_operands());

	auto ptr = main_stack.get();
	std::list<operand*> a;

	call_function(entry_point.get(), entry_point.get(), a, main_stack.get(), &ptr);
	

	std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
	std::chrono::duration<float> difference = now - old;

	main_stack.reset();
	main_stack = std::unique_ptr<function_stack>(ptr);


	//LOG("\n\n-----returning from: " << entry_point->def.identifier << "-----\n\n");

	for (auto& v : main_stack->variables) {
		v->get_lvalue()->print2();
	}
	std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";

	printf("\ntime taken: %.6f\n", difference.count());


}