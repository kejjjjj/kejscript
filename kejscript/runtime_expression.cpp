#include "pch.hpp"

#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "operators.hpp"

using NodeVector = std::list<std::unique_ptr<expression_node>>;

void create_operand(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& stack, NodeVector& nodes);
void create_operator(ListTokenPtr::iterator& it, expression_context& stack, NodeVector& nodes);

bool assign_unary_to_operand(ListTokenPtr::iterator& it, expression_context& context);
std::unique_ptr<expression_node> assign_identifier_to_operand(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context);
bool assing_postfix_to_operand(ListTokenPtr::iterator& it, expression_context& context);

[[nodiscard]] std::unique_ptr<expression_node> evaluate_expressions(NodeVector& nodes);



[[nodiscard]] std::unique_ptr<expression_results> evaluate_expression(ListTokenPtr::iterator it, ListTokenPtr::iterator end, const expression_token_stack& stack)
{
	expression_context ctx(stack);
	
	NodeVector nodes;

	if(it == end)
		return std::make_unique<expression_results>(it);


	while (it != end && it->get()->is_operator(punctuation_e::P_SEMICOLON) == false) {

		create_operand(it, end, ctx, nodes);

		if (ctx.stack.time_to_exit()) {
			it = ctx.stack.location;
			break;
		}

		create_operator(it, ctx, nodes);
	}
	
	if (nodes.size() == 1) {
		return std::make_unique<expression_results>(it, nodes.front());
	}

	auto result = evaluate_expressions(nodes);

	return std::make_unique<expression_results>(it, result);
}

void create_operand(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& ctx, NodeVector& nodes)
{
	while (assign_unary_to_operand(it, ctx));
	auto node = assign_identifier_to_operand(it, end, ctx);
	while (assing_postfix_to_operand(it, ctx));

	if (ctx.stack.time_to_exit() && ctx.expression.empty())
		return;

	ctx.expression.op.is_operator = false;

	if(!node)
		node = std::make_unique<expression_node>(ctx.expression);

	nodes.push_back(std::move(node));

	ctx.expression = expression_t();

}
void create_operator(ListTokenPtr::iterator& it, expression_context& ctx, NodeVector& nodes)
{
	if (it->get()->is_operator(punctuation_e::P_SEMICOLON))
		return;

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false) {
		ctx.stack.assign_to_stack_if_possible(it);

		if (ctx.stack.time_to_exit())
			return;
	}

	const auto token = dynamic_cast<punctuation_token_t*>(it->get());

	auto node = std::make_unique<expression_node>(expression_node::operator_s(token->punc, token->priority));
	nodes.push_back(std::move(node));

	std::advance(it, 1);
	return;
}
bool assign_unary_to_operand(ListTokenPtr::iterator& it, expression_context& context)
{

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return false;

	if (it->get()->is_punctuation() == false)
		return false;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());

	if (ptr->is_operator(P_PAR_OPEN)) //parentheses are the operand, so go to the next function
		return false;

	if (!is_unary_operator(ptr->punc)) {
		return false;
	}

	context.expression.prefix.push_back(it->get());

	std::advance(it, 1);
	return true;
}
std::unique_ptr<expression_node> assign_identifier_to_operand(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context)
{

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit()) {
		return nullptr;
	}

	if (it->get()->is_operator(P_PAR_OPEN)) {

		std::advance(it, 1);

		expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
		auto results = evaluate_expression(it, end, stack);
		
		it = results->it;
		context.stack.stack.num_close++;
		context.stack.location = it;
		
		std::advance(it, 1);
		return std::move(results->expression);

	}

	auto& expression = context.expression;
	expression.identifier = it->get();
	std::advance(it, 1);

	return nullptr;

}
bool assing_postfix_to_operand(ListTokenPtr::iterator& it, expression_context& context)
{

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return false;

	if (it->get()->is_punctuation() == false)
		return false;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());

	if (!is_postfix_operator(ptr->punc))
		return false;

	context.expression.postfix.push_back(it->get());

	std::advance(it, 1);


	return true;
}
//using ExpressionItr = std::vector<expression_t>::iterator;

void set_operator_priority(NodeVector::iterator& itr1, NodeVector::iterator& itr2, const NodeVector::iterator& end);

[[nodiscard]] std::unique_ptr<expression_node> evaluate_expressions(NodeVector& nodes)
{
	NodeVector::iterator itr1 = nodes.begin(), itr2;

	while (nodes.size() > 2) {
		itr1 = ++nodes.begin();
		itr2 = itr1;
		std::advance(itr2, 2);

		set_operator_priority(itr1, itr2, nodes.end());

		itr2 = itr1;

		const auto& punctuation = std::get<expression_node::operator_s>(itr1->get()->_op).punc;
		auto& left_operand = *--itr1;
		auto& right_operand = *++itr2;

		auto function = evaluation_functions::getInstance().find_function(punctuation);

		if (function.has_value() == false)
			throw runtime_error("unsupported operator");

		*itr2 = function.value()(*left_operand, *right_operand);

		nodes.erase(itr1, itr2);
	}

	return std::move(*nodes.begin());

}

void set_operator_priority(NodeVector::iterator& itr1, NodeVector::iterator& itr2, const NodeVector::iterator& end)
{
	OperatorPriority op{}, next_op{};

	if (itr2 == end)
		return;

	do {

		op = std::get<expression_node::operator_s>(itr1->get()->_op).priority;
		next_op = std::get<expression_node::operator_s>(itr2->get()->_op).priority;

		if (itr2 == end || std::prev(itr2) == end || next_op <= op)
			break;

		std::advance(itr1, 2);
		std::advance(itr2, 2);

	} while (next_op > op && itr2 != end);
	
}