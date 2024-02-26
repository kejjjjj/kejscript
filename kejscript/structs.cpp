#include "structs.hpp"

std::unique_ptr<operand> call_constructor(
	struct_call* callee,
	const std::list<operand*>& args,
	[[maybe_unused]] struct function_stack* stack)
{

	LOG("callee: " << callee << '\n');


	auto unique_stack = std::make_unique<function_stack>(); //no need to manually reset this
	auto stack_ptr = unique_stack.get();
	auto arg = args.begin();
	auto function = callee->target;

	if (function) {

		//create the stack
		size_t num_vars = function->def.num_operands();

		for (size_t i = 0; i < num_vars; ++i) {
			stack_ptr->variables.push_back(std::make_shared<operand>());
			stack_ptr->variables[i]->value->set_value(bool_dt(false));
		}


		//create arguments
		size_t i = 0;
		for ([[maybe_unused]] const auto& _ : function->def.parameters) {
			evaluation_functions::assign_to_lvalue(stack_ptr->variables[i]->get_lvalue(), *arg);
			++arg;
			++i;
		}

		////create local variables
		//for (auto& v : function->def.variables) {
		//	stack_ptr->variables[i].get_lvalue()->identifier = v;
		//}
	}

	stack_ptr->_this = std::make_shared<object>();
	stack_ptr->_this->is_struct = true;

	for (auto& member : callee->target_struct->initializers)
		stack_ptr->_this->insert(member->variable);


	size_t i = 0;
	for (auto& member : callee->target_struct->initializers) {

		if (member->initializer) {
			auto result = evaluate_expression(function, stack_ptr, member->initializer->expression_ast);
			evaluation_functions::assign_to_lvalue(stack_ptr->_this->variables[i], result);
			
		}

		++i;
	}

	if (function) {
		for (auto& instruction : function->instructions) {

			if (instruction->execute(stack_ptr)) {
				break;
			}

		}
	}

	auto v = std::make_shared<variable>();
	v->obj = std::move(stack_ptr->_this);
	v->obj->structure = callee->target_struct;

	return 0;



}

std::unique_ptr<operand> call_method(
	function_block* callee,
	std::list<operand*>& args,
	std::shared_ptr<object>& _this)
{

	LOG("callee: " << callee << '\n');


	auto unique_stack = std::make_unique<function_stack>(); //no need to manually reset this
	auto stack_ptr = unique_stack.get();
	auto arg = args.begin();
	auto function = callee;

	//create the stack
	size_t num_vars = function->def.num_operands();

	for (size_t i = 0; i < num_vars; ++i) {
		stack_ptr->variables.push_back(std::make_shared<operand>());
		stack_ptr->variables[i]->value->set_value(bool_dt(false));
	}


	size_t i = 0;
	for ([[maybe_unused]] const auto& _ : function->def.parameters) {
		evaluation_functions::assign_to_lvalue(stack_ptr->variables[i]->get_lvalue(), *arg);
		stack_ptr->variables[i]->value->set_value(bool_dt(false));

		++arg;
		++i;
	}

	////create local variables
	//for (auto& v : function->def.variables) {
	//	stack_ptr->variables[i].get_lvalue()->identifier = v;
	//}
	
	stack_ptr->_this = _this;

	for (auto& instruction : function->instructions) {

		if (instruction->execute(stack_ptr)) {
			break;

			//auto& returned_value = stack_ptr->return_value; //claim ownership!

			//if (returned_value && !returned_value->valueless_type())
			//	returned_value->lvalue_to_rvalue();

			//return nullptr;
			//return returned_value;

		}

	}
	
	return nullptr;

}