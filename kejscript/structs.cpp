#include "structs.hpp"

std::unique_ptr<operand> call_constructor(
	struct_call* callee,
	const std::list<std::unique_ptr<operand>>& args,
	[[maybe_unused]] struct function_stack* stack)
{

	LOG("callee: " << callee << '\n');


	auto unique_stack = std::make_unique<function_stack>(); //no need to manually reset this
	auto stack_ptr = unique_stack.get();
	auto arg = args.begin();
	auto function = callee->target;

	if (function) {
		//create arguments
		for (const auto& param : function->def.parameters) {
			stack_ptr->variables.push_back(std::make_shared<variable>(param));
			evaluation_functions::assign_to_lvalue(stack_ptr->variables.back(), *arg);
			++arg;
		}

		//create local variables
		for (auto& v : function->def.variables) {
			stack_ptr->variables.push_back(std::make_shared<variable>(v));
		}
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

	return create_lvalue(v);



}

std::unique_ptr<operand> call_method(
	function_block* callee,
	const std::list<std::unique_ptr<operand>>& args,
	std::shared_ptr<object>& _this)
{

	LOG("callee: " << callee << '\n');


	auto unique_stack = std::make_unique<function_stack>(); //no need to manually reset this
	auto stack_ptr = unique_stack.get();
	auto arg = args.begin();
	auto function = callee;


	for (const auto& param : function->def.parameters) {
		stack_ptr->variables.push_back(std::make_shared<variable>(param));
		evaluation_functions::assign_to_lvalue(stack_ptr->variables.back(), *arg);
		++arg;
	}

	//create local variables
	for (auto& v : function->def.variables) {
		stack_ptr->variables.push_back(std::make_shared<variable>(v));
	}
	
	stack_ptr->_this = _this;

	for (auto& instruction : function->instructions) {

		if (instruction->execute(stack_ptr)) {

			auto returned_value = std::unique_ptr<operand>(function->return_value); //claim ownership!

			if (!returned_value->is_object())
				returned_value->lvalue_to_rvalue();

			function->return_value = 0;

			return returned_value;

		}

	}
	
	return nullptr;

}