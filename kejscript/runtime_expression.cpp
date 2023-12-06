#include "pch.hpp"

#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "operators.hpp"

using NodeVector = std::vector<std::unique_ptr<expression_node>>;

void create_operand(VectorTokenPtr::iterator& it, expression_context& stack, NodeVector& nodes);
void create_operator(VectorTokenPtr::iterator& it, expression_context& stack, NodeVector& nodes);

bool assign_unary_to_operand(VectorTokenPtr::iterator& it, expression_context& context);
void assign_identifier_to_operand(VectorTokenPtr::iterator& it, expression_context& context);
bool assing_postfix_to_operand(VectorTokenPtr::iterator& it, expression_context& context);

void evaluate_expressions(NodeVector& nodes);



[[nodiscard]] VectorTokenPtr::iterator evaluate_expression(VectorTokenPtr::iterator it, VectorTokenPtr::iterator end, const expression_token_stack& stack)
{
	expression_context ctx(stack);
	
	NodeVector nodes;


	while (it != end && it->get()->is_operator(punctuation_e::P_SEMICOLON) == false) {

		create_operand(it, ctx, nodes);

		if (ctx.stack.time_to_exit()) {
			it = ctx.stack.location;
			break;
		}

		create_operator(it, ctx, nodes);
	}
	
	evaluate_expressions(nodes);
	return it;
}

void create_operand(VectorTokenPtr::iterator& it, expression_context& ctx, NodeVector& nodes)
{
	while (assign_unary_to_operand(it, ctx));
	assign_identifier_to_operand(it, ctx);
	while (assing_postfix_to_operand(it, ctx));

	if (ctx.stack.time_to_exit() && ctx.expression.empty())
		return;

	ctx.expression.op.is_operator = false;
	ctx.expressions.push_back(ctx.expression);

	auto node = std::make_unique<expression_node>(ctx.expression);
	nodes.push_back(std::move(node));

	ctx.expression = expression_t();

}
void create_operator(VectorTokenPtr::iterator& it, expression_context& ctx, NodeVector& nodes)
{
	if (it->get()->is_operator(punctuation_e::P_SEMICOLON))
		return;

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false) {
		ctx.stack.assign_to_stack_if_possible(it);

		if (ctx.stack.time_to_exit())
			return;
	}

	const auto token = dynamic_cast<punctuation_token_t*>(it->get());

	expression_t expression;
	expression.op.is_operator = true;
	expression.op.punc = token->punc;
	expression.op.priority = token->priority;

	expression.identifier = token;
	ctx.expressions.push_back(expression);

	auto node = std::make_unique<expression_node>(expression_node::operator_s{.punc = token->punc, .priority = token->priority });
	nodes.push_back(std::move(node));

	std::advance(it, 1);
	return;
}
bool assign_unary_to_operand(VectorTokenPtr::iterator& it, expression_context& context)
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
void assign_identifier_to_operand(VectorTokenPtr::iterator& it, expression_context& context)
{

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit()) {
		return;
	}

	if (it->get()->tt > tokentype_t::FLOAT_LITERAL)
		throw runtime_error(it->get(), "don't use that yet wtf");

	auto& expression = context.expression;
	expression.identifier = it->get();
	std::advance(it, 1);

}
bool assing_postfix_to_operand(VectorTokenPtr::iterator& it, expression_context& context)
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

void evaluate_expressions(NodeVector& nodes)
{
	NodeVector::iterator itr1, itr2;

	while (nodes.size() > 2) {
		itr1 = ++nodes.begin();
		itr2 = itr1;
		std::advance(itr2, 2);

		set_operator_priority(itr1, itr2, nodes.end());

		itr2 = itr1;

		auto& punctuation = itr1->get()->_operator.punc;
		auto& left_operand = *--itr1;
		auto& right_operand = *++itr2;

		auto function = evaluation_functions::getInstance().find_function(punctuation);

		function.value()(*left_operand, *right_operand);

		nodes.erase(itr1, itr2);
	}
}

void set_operator_priority(NodeVector::iterator& itr1, NodeVector::iterator& itr2, const NodeVector::iterator& end)
{
	OperatorPriority op{}, next_op{};

	if (itr2 != end) {
		do {

			op = itr1->get()->_operator.priority;
			next_op = itr2->get()->_operator.priority;

			if (itr2 == end || itr2 == (end - 1) || next_op <= op)
				break;

			std::advance(itr1, 2);
			std::advance(itr2, 2);

		} while (next_op > op);
	}
}