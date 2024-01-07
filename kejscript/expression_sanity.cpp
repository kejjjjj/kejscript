#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

void peek_unary_operator(ListTokenPtr::iterator & it, ListTokenPtr::iterator & end, expression_context & context);
void peek_identifier(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context, std::unique_ptr<expression_block>& block);
void peek_postfix_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context, singular* s);
std::unique_ptr<ast_node> generate_ast(singularlist& operands, operatorlist& operators);


validation_expression::literal create_literal(ListTokenPtr::iterator& token) {
	validation_expression::literal literal;
	literal.value = *(*token)->value.get();

	switch (token->get()->tt) {
	case tokentype_t::NUMBER_LITERAL:
		literal.type = validation_expression::literal::NUMBER_LITERAL;
		break;
	case tokentype_t::FLOAT_LITERAL:
		literal.type = validation_expression::literal::FLOAT_LITERAL;
		break;
	case tokentype_t::_TRUE:
		literal.type = validation_expression::literal::_TRUE;
		break;
	case tokentype_t::_FALSE:
		literal.type = validation_expression::literal::_FALSE;
		break;
	}

	return literal;
}

void tokenize_operand(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context)
{
	auto s = std::make_unique<singular>();

	std::list<_operator> prefix, postfix;
	const auto& current_function = linting_data::getInstance().current_function;

	peek_unary_operator(it, end, context);

	ListTokenPtr::iterator identifier_it = it;
	context.identifier = it->get()->string;

	peek_identifier(it, end, context, context.block);
	peek_postfix_operator(it, end, context, s.get());

	//an expression with parentheses is the next one
	if (context.block->next) {
		s->parentheses = std::move(context.block->next->ast_tree);
		context.ex.push_back(std::move(s));
		context.block->next.reset();
		return;
	}

	// a literal token
	if (identifier_it->get()->is_identifier() == false) {
		s->make_literal(create_literal(identifier_it));
	}
	else {
		validation_expression::other other{  
			identifier_it->get()->string, 
			current_function->get_index_for_variable(identifier_it->get()->string)};

		s->make_other(other);
	}

	s->owner = linting_data::getInstance().current_function;
	s->token = identifier_it->get();
	context.ex.push_back(std::move(s));
}

#include "operators.hpp"
void tokenize_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& ctx)
{
	if (it == end || it->get()->is_operator(punctuation_e::P_SEMICOLON))
		return;

	if (ctx.stack.time_to_exit())
		return;
	
	const auto token = it->get();

	if(token->is_punctuation() == false)
		throw linting_error(token, "expected the expression to end before '%s'", token->string.c_str());

	auto op = dynamic_cast<punctuation_token_t*>(token);

	if (!satisfies_operator(op->punc)) {
		throw linting_error(token, "expected the expression to end before '%s'", token->string.c_str());
	}

	auto func = evaluation_functions::find_function(op->punc);
	ctx.operators.push_back({ op->priority, op->punc, reinterpret_cast<void*>(func), operator_type::STANDARD, it->get()});
	std::advance(it, 1);
	return;
}
[[nodiscard]] l_expression_results evaluate_expression_sanity(
	ListTokenPtr::iterator it, 
	ListTokenPtr::iterator end, 
	std::unique_ptr<expression_block>& _block, 
	const expression_token_stack& stack)
{
	if (it == end)
		throw linting_error(it->get(), "an empty expression is not allowed");

	expression_context ctx(stack);
	
	auto& expressions = ctx.ex;
	auto& operators = ctx.operators;
	ctx.block = std::move(_block);
	auto& block = ctx.block;

	while (it != end) {

		tokenize_operand(it, end, ctx);

		//the comma should only exit if we are not using the stack
		if (it->get()->is_operator(punctuation_e::P_COMMA)) {
			
			//this condition means that we are currently parsing function arguments
			if (ctx.stack.not_in_use() == false && ctx.stack.opening == P_PAR_OPEN)
			{
				//this expression block ends here
				if (expressions.empty()) {
					throw linting_error(it->get(), "an empty expression is not allowed");
				}

				block->ast_tree = generate_ast(expressions, operators);
				block->next = std::make_unique<expression_block>();

				++it; //skip the comma
				ctx.stack.num_evaluations += 1;
				return evaluate_expression_sanity(it, end, block->next, ctx.stack);
			}

			//a random comma
			if (ctx.stack.num_evaluations == 0)
				throw linting_error(it->get(), "the ',' operator is not allowed here");

			break;
		}

		if (ctx.stack.time_to_exit()) {
			it = ctx.stack.location;
			LOG("returning to " << it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");
			break;
		}

		tokenize_operator(it, end, ctx);

		if (it->get()->is_operator(punctuation_e::P_SEMICOLON)) {
			break;
		}
	}

	if (expressions.empty()) {
		throw linting_error(it->get(), "an empty expression is not allowed");
	}

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false)
		throw linting_error(it->get(), "expected to find a '%s'\n", punctuations[ctx.stack.closing].identifier.c_str());


	//generate the abstract syntax tree
	block->ast_tree = generate_ast(expressions, operators);

	//if there were parentheses in the expression, insert them into the leaves

	_block = std::move(ctx.block);
	return { it, ctx.stack.num_evaluations + 1 };
}

void peek_unary_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context)
{
	if (it == end)
		return;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return;

	if (it->get()->is_punctuation() == false)
		return;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());
	
	if (ptr->is_operator(P_PAR_OPEN)) //parentheses are the operand, so go to next function
		return;

	if (bad_unary_operator(ptr->punc))
		throw linting_error(it->get(), "expected an expression instead of '%s'", ptr->string.c_str());

	if (!is_unary_operator(ptr->punc)) {
		return;
	}
	auto func = evaluation_functions::find_unary_function(ptr->punc);

	context.operators.push_back({ .priority = ptr->priority, .punc = ptr->punc, .eval=reinterpret_cast<void*>(func), .type = operator_type::UNARY});
	return peek_unary_operator(++it, end, context);
}
void peek_identifier(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context, std::unique_ptr<expression_block>& block)
{
	if (it == end)
		return;

	context.stack.assign_to_stack_if_possible(it); //should only be true when the current token is '('

	if (context.stack.time_to_exit()) {
		return;
	}

	if (it->get()->is_punctuation()) {
		if (it->get()->is_operator(P_PAR_OPEN)) {

			int random = rand() % 127;
			std::cout << "eval start " << random << '\n';
			
			std::advance(it, 1);
			expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
			block->next = std::make_unique<expression_block>();
			auto results = evaluate_expression_sanity(it, end, block->next, stack);
			if (results.num_evaluations > 1) {
				throw linting_error(results.it->get(), "expected one expression instead of %u", (unsigned __int64)results.num_evaluations);
			}
			std::cout << "eval end " << random << '\n';
			it = results.it;

			context.stack.stack.num_close++;
			context.stack.location = it;

			std::advance(it, 1);
			return;
		}

		throw linting_error(it->get(), "expected an identifier instead of '%s'", it->get()->string.c_str());

	}
	
	auto token = it->get();

	//test if it's an identifier
	if (token->tt == tokentype_t::IDENTIFIER) {

		//make sure it exists in the stack before it's used
		auto scope = linting_data::getInstance().active_scope;
		auto& data = linting_data::getInstance();
		if (scope->variable_exists(token->string) == false && !data.function_exists(token->string)) {
			throw linting_error(token, "the identifier '%s' is undefined", token->string.c_str());
		}

	}

	if (token->is_reserved_keyword())
		throw linting_error(token, "the '%s' keyword is a reserved keyword and it cannot be used here", token->string.c_str());


	std::advance(it, 1);
	return;
}

void peek_postfix_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, expression_context& context, singular* s)
{
	if (it == end)
		return;

	context.stack.assign_to_stack_if_possible(it);
	if (context.stack.time_to_exit())
		return;

	if (it->get()->is_punctuation() == false)
		return;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());
	
	if (!is_postfix_operator(ptr->punc))
		return;

	_operator v = { .priority = ptr->priority, .punc = ptr->punc };

	if (ptr->punc == P_BRACKET_OPEN) {
		it = evaluate_subscript_sanity(it, end, context);
	}
	else if (ptr->punc == P_PAR_OPEN) {
		it = evaluate_function_call_sanity(it, end, context, s);
	}
	else
		it = linting_data::getInstance().tokens->erase(it);

	context.operators.push_back({ .priority = ptr->priority, .punc = ptr->punc, .eval = nullptr, .type= operator_type::POSTFIX});

	return peek_postfix_operator(++it, end, context, s);
}
struct precedence_results
{
	operatorlist::iterator position;
	size_t index = 0; //number of operands skipped

};
precedence_results get_lowest_precedence(operatorlist::iterator& itr1, const operatorlist::iterator& end);
void generate_ast_recursive(std::unique_ptr<ast_node>& node, singularlist& expressions, operatorlist& operators);

void generate_ast_operand_leaf(std::unique_ptr<ast_node>& node, singularlist& expressions, operatorlist& operators)
{
	auto& front = expressions.front();

	if (front->parentheses) {
		node = std::unique_ptr<ast_node>(front->parentheses.release());
		expressions.clear();
		return;
	}

	node->make_operand(front);
	expressions.clear();

	if (operators.size()) {
		if (operators.front().type == operator_type::UNARY) {
			node->left = std::make_unique<ast_node>();
			generate_ast_recursive(node->left, expressions, operators);
		}
		else {
			node->right = std::make_unique<ast_node>();
			generate_ast_recursive(node->right, expressions, operators);
		}
	}
}
void generate_ast_recursive(std::unique_ptr<ast_node>& node, singularlist& expressions, operatorlist& operators)
{
	singularlist::iterator itr1;
	operatorlist::iterator op1;

	if (expressions.empty()) {

		if(operators.empty())
			return;

		if (operators.size() == 1 && operators.front().type != operator_type::POSTFIX) {
			throw linting_error(operators.front().token, "the expression ended unexpectedly");
		}
	}

	op1 = operators.begin();
	auto op = get_lowest_precedence(op1, operators.end());

	if (operators.size() && op.position->type == operator_type::UNARY) {
		node->make_operator(*op.position);
		node->left = std::make_unique<ast_node>();
		operators.erase(op.position);
		return generate_ast_recursive(node->left, expressions, operators);
	}

	if (expressions.size() == 1) {
		return generate_ast_operand_leaf(node, expressions, operators);
	}

	itr1 = expressions.begin();
		
	std::advance(itr1, op.index);
	const auto& left_substr = itr1;
	const auto& right_substr = std::next(itr1);

	node->left = std::make_unique<ast_node>();
	node->right = std::make_unique<ast_node>();
	node->make_operator(*op.position);

	//sort the left tree (everything to the left of the operator)
	singularlist left_branch = singularlist(std::make_move_iterator(expressions.begin()), std::make_move_iterator(std::next(left_substr)));
	operatorlist left_branch_op = operatorlist(operators.begin(), op.position);
	generate_ast_recursive(node->left, left_branch, left_branch_op);

	//sort the right tree (everything to the right of the operator)
	singularlist right_branch = singularlist(std::make_move_iterator(right_substr), std::make_move_iterator(expressions.end()));
	operatorlist right_branch_op = operatorlist(std::next(op.position), operators.end());
	generate_ast_recursive(node->right, right_branch, right_branch_op);

}

std::unique_ptr<ast_node> generate_ast(singularlist& operands, operatorlist& operators)
{
	nodeptr root = std::make_unique<ast_node>();
	generate_ast_recursive(root, operands, operators);
	return std::move(root);

}
precedence_results get_lowest_precedence(operatorlist::iterator& itr1, const operatorlist::iterator& end)
{
	OperatorPriority priority{};
	OperatorPriority lowest_precedence = itr1->priority;

	operatorlist::iterator lowest_precedence_i = itr1;
	precedence_results results{ itr1, 0 };
	
	while (itr1 != end && itr1->type != operator_type::STANDARD) {
		std::cout << "hiii\n";
		std::advance(itr1, 1);
	}

	if (itr1 == end)
		return results;

	size_t real_distance = 0;
	size_t lowest_distance = 0;
	do {
		priority = itr1->priority;
		
		if (priority < lowest_precedence) {
			lowest_precedence = priority;
			lowest_precedence_i = itr1;
			lowest_distance = real_distance;
		}

		std::advance(itr1, 1);

		while (itr1 != end && itr1->type != operator_type::STANDARD) {
			std::cout << "hii\n";
			std::advance(itr1, 1);
		}

		if(itr1 != end)
			real_distance++;

	} while (itr1 != end);

	std::advance(results.position, real_distance);
	results.index = lowest_distance;
	results.position = lowest_precedence_i;
	return results;
}