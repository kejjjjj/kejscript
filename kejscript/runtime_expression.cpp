#include "pch.hpp"

#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "operators.hpp"
#include "function.hpp"
#include "structs.hpp"

bool expression_block::execute(function_stack* stack)
{
	evaluate_expression(owner, stack, expression_ast);
	return false;
}
static void evaluate_unary(operand_ptr& target, const nodeptr& node)
{
	if (node->is_unary() == false)
		return;

	auto& op = node->get_operator();

	target = std::move(reinterpret_cast<evaluation_functions::unaryfuncptr>(op->eval)(target));
	return evaluate_unary(target, node->left);

}
static void evaluate_postfix(function_block* owner, function_stack* stack, operand_ptr& target, const nodeptr& node)
{
	if (node->is_postfix() == false)
		return;

	auto& op = node->get_operator();
	
	if (op->block->type() == code_block_e::FN_CALL) {

		auto function = dynamic_cast<function_call*>(op->block.get());

		expression_block* arg = function->arguments.get();
		std::list<std::unique_ptr<operand>> args;
		size_t argc = 1;
		while (arg) {
			auto expr = evaluate_expression(owner, stack, arg->expression_ast);

			if (!expr) {
				std::string s_argc = std::to_string(argc);
				throw runtime_error(target->_operand, "attempted to pass a null value to function (argument %s)", s_argc.c_str());
			}
			args.push_back(std::move(expr));
			arg = arg->next.get();
			++argc;
		}
		target = call_function(owner, function->target, args, stack);

		if (!target) //function didn't return a value
			return;

	}
	else if (op->block->type() == code_block_e::STRUCT_CALL) {

		auto function = dynamic_cast<struct_call*>(op->block.get());

		expression_block* arg = function->arguments.get();
		std::list<std::unique_ptr<operand>> args;
		size_t argc = 1;
		while (arg) {
			auto expr = evaluate_expression(owner, stack, arg->expression_ast);

			if (!expr) {
				std::string s_argc = std::to_string(argc);
				throw runtime_error(target->_operand, "attempted to pass a null value to function (argument %s)", s_argc.c_str());
			}
			args.push_back(std::move(expr));
			arg = arg->next.get();
			++argc;
		}

		target = call_constructor(function, args, stack);

		if (!target) //function didn't return a value (should never happen?)
			return;

	}
	else if (op->block->type() == code_block_e::EXPRESSION) { //subscript

		auto expression = evaluate_expression(owner, stack, node->right);
		target = subscript(target, expression);


	}
	else if (op->block->type() == code_block_e::MEMBER_ACCESS) {
		auto member = dynamic_cast<member_access_block*>(op->block.get());

		auto obj = target->is_object();

		if (!obj || !obj->structure)
			throw runtime_error(node->get_token(), "operand is not a struct");

		auto var = obj->structure->quick_lookup[member->member].var_index;

		if (var == -1) {
			std::string wtf = member->member;
			throw runtime_error(node->get_token(), "'%s' is not a member of '%s'", wtf.c_str(), obj->structure->identifier.c_str());
		}

		target = create_lvalue(obj->variables[var]);

	}

	target->_operand = op->token;

	return;

	//return evaluate_postfix(owner, stack, target, node->left);

}
std::unique_ptr<operand> evaluate_initializer_list(function_block* owner, function_stack* stack, const initializer_list* list)
{
	auto rvalue_array = std::make_shared<variable>();
	rvalue_array->initialized = true;
	rvalue_array->identifier = "list";
	rvalue_array->obj = std::make_shared<object>();

	auto current = list->expression.get();
	while (current) {

		const nodeptr& target = current->list ? current->list->expression->expression_ast : current->expression_ast;
		auto result = evaluate_expression(owner, stack, target);
		rvalue_array->obj->insert(result);
		current = current->next.get();
	}

	return create_lvalue(rvalue_array);
}
std::unique_ptr<operand> evaluate_expression(function_block* owner, function_stack* stack, const nodeptr& node)
{
	if (node->is_list()) {
		auto ret = evaluate_initializer_list(owner, stack, node->get_list().get());
		ret->_operand = std::get<std::unique_ptr<singular>>(node->contents)->token;
		return ret;
	}

	if (node->is_postfix()) {
		auto expression = evaluate_expression(owner, stack, node->left);

		if(!expression)
			throw runtime_error(node->left->get_token(), "null value in postfix expression");


		evaluate_postfix(owner, stack, expression, node);
		return std::move(expression);
	}

	if (node->is_unary()) {
		auto expression = evaluate_expression(owner, stack, node->left);
		if (!expression)
			throw runtime_error(node->left->get_token(), "null value in unary expression");
		evaluate_unary(expression, node);
		return std::move(expression);
	}

	if (node->is_leaf()) {

		auto& leaf = node->get_operand();	
		return std::make_unique<operand>(*leaf.get(), stack);
	}

	auto left_operand = evaluate_expression(owner, stack, node->left);
	auto right_operand = evaluate_expression(owner, stack, node->right);

	if (!left_operand || !right_operand) {
		throw runtime_error(node->get_token(), "the operand does not have a value");
	}

	auto& op = node->get_operator();

	if (
		op->punc != P_ASSIGN &&
		op->punc != P_EQUALITY &&
		op->punc != P_UNEQUALITY
		&& left_operand->is_compatible_with(*right_operand.get()) == false) {
		throw runtime_error(left_operand->_operand, "an operand of type '%s' is not compatible with an operand of type '%s'",
			left_operand->get_type().c_str(), right_operand->get_type().c_str());
	}

	auto ret = reinterpret_cast<evaluation_functions::funcptr>(op->eval)(*left_operand, *right_operand);

	ret->_operand = node->get_token();

	//if(right_operand->_operand)
	//	ret->_operand = right_operand->_operand;
	//else if (left_operand->_operand)
	//	ret->_operand = left_operand->_operand;

	return ret;
}
