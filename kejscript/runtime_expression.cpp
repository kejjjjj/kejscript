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
static void evaluate_unary(operand* target, const nodeptr& node)
{
	if (node->is_unary() == false)
		return;

	auto& op = node->get_operator();

	target = std::move(reinterpret_cast<evaluation_functions::unaryfuncptr>(op->eval)(target));
	return evaluate_unary(target, node->left);

}

static std::list<operand*> parse_arguments(function_block* owner, function_stack* stack, operand* target, std::unique_ptr<expression_block>& arguments)
{
	expression_block* arg = arguments.get();
	std::list<operand*> args;
	size_t argc = 1;
	while (arg) {
		auto expr = evaluate_expression(owner, stack, arg->expression_ast);

		if (!expr) {
			std::string s_argc = std::to_string(argc);
			throw runtime_error(target->_operand, "attempted to pass a null value to function (argument %s)", s_argc.c_str());
		}
		args.push_back(expr);
		arg = arg->next.get();
		++argc;
	}

	return (args);

}

static void evaluate_postfix(function_block* owner, function_stack* stack, operand* target, const nodeptr& node)
{
	if (node->is_postfix() == false)
		return;

	auto& op = node->get_operator();
	
	if (op->block->type() == code_block_e::FN_CALL) {

		auto function = dynamic_cast<function_call*>(op->block.get());

		auto target_func = target->is_function_pointer();

		if (!target_func) {
			throw runtime_error(node->get_token(), "the operand is not a function");
		}

		auto args = parse_arguments(owner, stack, target, function->arguments);
		//auto& v = target->get_lvalue();

		target = 0;

		//if (v->member) {
		//	target = call_method(target_func, args, v->member);
		//}
		//else {
		//	target = call_function(owner, target_func, args, stack);
		//}
		if (!target) //function didn't return a value
			return;

	}
	else if (op->block->type() == code_block_e::STRUCT_CALL) {

		//auto function = dynamic_cast<struct_call*>(op->block.get());
		
		target = 0;
		//target = call_constructor(function, std::move(parse_arguments(owner, stack, target, function->arguments)), stack);

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

			var = obj->structure->quick_method_lookup[member->member].var_index; //test if it's a method

			if (var == -1) {
				std::string wtf = member->member;
				throw runtime_error(node->get_token(), "'%s' is not a member of '%s'", wtf.c_str(), obj->structure->identifier.c_str());
			}

			auto& v = target->get_lvalue();

			v->function_pointer = obj->structure->methods[var].get();
			v->member = obj;
		}
		else {
			obj->variables[var]->member = obj;
			target->get_lvalue() = obj->variables[var];
		}


	}

	target->_operand = op->token;

	return;

	//return evaluate_postfix(owner, stack, target, node->left);

}
static operand* evaluate_initializer_list(function_block* owner, function_stack* stack, const initializer_list* list)
{

	throw runtime_error(list->expression->expression_ast->get_token(), "null value in postfix expression");


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



	return nullptr;
	//return create_lvalue(rvalue_array);
}
operand* evaluate_expression(function_block* owner, function_stack* stack, const nodeptr& node)
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
		return (expression);
	}

	if (node->is_unary()) {
		auto expression = evaluate_expression(owner, stack, node->left);
		if (!expression)
			throw runtime_error(node->left->get_token(), "null value in unary expression");
		evaluate_unary(expression, node);
		return (expression);
	}

	if (node->is_leaf()) {

		auto& leaf = node->get_operand();

		if (leaf->v.type == validation_expression::Type::LITERAL) {

			auto& ptr = runtime::literals[leaf->get_literal().literal_index];

			//std::cout << "returning: " << leaf->get_literal().literal_index << '\n';

			return ptr.get();
		}

		return stack->variables[leaf->get_other().operand_index].get();
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
		&& left_operand->is_compatible_with(*right_operand) == false) {
		throw runtime_error(left_operand->_operand, "an operand of type '%s' is not compatible with an operand of type '%s'",
			left_operand->get_type().c_str(), right_operand->get_type().c_str());
	}

	auto ret = reinterpret_cast<evaluation_functions::funcptr>(op->eval)(left_operand, right_operand);
	ret->_operand = node->get_token();
	return ret;
}
