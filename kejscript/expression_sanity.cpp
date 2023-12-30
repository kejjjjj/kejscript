#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"


std::optional<_operator> peek_unary_operator(ListTokenPtr::iterator & it, ListTokenPtr::iterator & end, l_expression_context & context);
bool peek_identifier(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, l_expression_context& context);
std::optional<_operator> peek_postfix_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, l_expression_context& context);
std::unique_ptr<ast_node> generate_ast_tree(std::list<singular>& expressions);


void tokenize_operand(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, l_expression_context& context, std::list<singular>& ex)
{

	std::list<_operator> prefix, postfix;

	while (auto v = peek_unary_operator(it, end, context)) {
		prefix.push_back(v.value());
	}

	ListTokenPtr::iterator identifier_it = it;
	if (peek_identifier(it, end, context) == false)
		throw linting_error(it->get(), "expected an identifier instead of '%s'", it->get()->string.c_str());

	while (auto v = peek_postfix_operator(it, end, context)) {
		postfix.push_back(v.value());
	}

	if (context.stack.time_to_exit() && context.expression.identifier == nullptr) //if identifier == '('
		return;

	// a literal token
	if (identifier_it->get()->is_identifier() == false) {
		validation_expression::literal literal;
		literal.value = *identifier_it->get()->value.get();
		switch (identifier_it->get()->tt) {
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
		singular expression(literal);
		expression.token = identifier_it->get();
		expression.location = identifier_it;
		ex.push_back(std::move(expression));

	}
	else {
		validation_expression::other other{ prefix, postfix, context.expression.identifier->string, identifier_it->get()->variable_index };
		singular expression(other);
		expression.token = identifier_it->get();
		expression.location = identifier_it;

		ex.push_back(std::move(expression));
	}

	context.expression = linting_expression();
}
void tokenize_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, l_expression_context& ctx, std::list<singular>& ex)
{
	if (it == end || it->get()->is_operator(punctuation_e::P_SEMICOLON))
		return;

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false) {
		ctx.stack.assign_to_stack_if_possible(it);

		if (ctx.stack.time_to_exit())
			return;
	}

	const auto token = it->get();

	if(token->is_punctuation() == false)
		throw linting_error(token, "expected the expression to end before '%s'", token->string.c_str());

	auto op = dynamic_cast<punctuation_token_t*>(token);

	if (!satisfies_operator(op->punc)) {
		throw linting_error(token, "expected the expression to end before '%s'", token->string.c_str());
	}

	singular s({op->priority, op->punc});

	s.token = it->get();
	s.location = it;

	ex.push_back(std::move(s));

	std::advance(it, 1);
	return;

}
[[nodiscard]] l_expression_results evaluate_expression_sanity(ListTokenPtr::iterator it, ListTokenPtr::iterator end, const expression_token_stack& stack)
{
	if (it == end)
		throw linting_error(it->get(), "an empty expression is not allowed");

	l_expression_context ctx(stack);

	auto block = std::make_unique<expression_block>(nullptr);
	block->start = it;
	auto first_token = it->get();
	std::list<singular> expressions;


	while (it != end) {

		tokenize_operand(it, end, ctx, expressions);

		if (ctx.undefined_var) {
			if (ctx.undefined_var->function == false) //undefined local variables will always error because they cannot be globals
				throw linting_error(ctx.undefined_var->location->get(), "'%s' is undefined", ctx.undefined_var->identifier.c_str());
			else //undefined variables will be pushed to a list that will be checked after validating the entire file
				linting_data::getInstance().undefined_variables.push_back(*ctx.undefined_var.get());
		}

		//the comma should only exit if we are not using the stack
		if (it->get()->is_operator(punctuation_e::P_COMMA)) {
			
			//this condition means that we are currently parsing function arguments or we come across a random comma in the middle of parentheses
			if (ctx.stack.not_in_use() == false && ctx.stack.opening == P_PAR_OPEN)
			{
				//this expression block ends here
				block->end = std::prev(it);

				if (expressions.empty()) {
					throw linting_error(it->get(), "an empty expression is not allowed");
				}

				block->ast_tree = generate_ast_tree(expressions);
				first_token->block = std::move(block);

				++it; //skip the comma
				//TODO: remove the comma token from the list
				ctx.stack.num_evaluations += 1;

				return evaluate_expression_sanity(it, end, ctx.stack);
			}

			if (ctx.stack.num_evaluations == 0)
				throw linting_error(it->get(), "the ',' operator is not allowed here");

			break;
		}

		if (ctx.stack.time_to_exit()) {
			it = ctx.stack.location;
			LOG("returning to " << it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");
			break;
		}

		tokenize_operator(it, end, ctx, expressions);

		if (it->get()->is_operator(punctuation_e::P_SEMICOLON)) {
			break;
		}
	}

	if (expressions.empty()) {
		throw linting_error(it->get(), "an empty expression is not allowed");
	}

	if (expressions.back().type == singular::Type::OPERATOR) {
		throw linting_error(it->get(), "an expression must not end at an operator");
	}

	if (ctx.stack.stack.num_open && ctx.stack.time_to_exit() == false)
		throw linting_error(it->get(), "expected to find a '%s'\n", punctuations[ctx.stack.closing].identifier.c_str());


	block->end = (it);

	//generate the abstract syntax tree
	block->ast_tree = generate_ast_tree(expressions);

	token_t* target = first_token;
	target->block = std::move(block);

	LOG(std::format("creating an expression from [{}, {}] to [{}, {}]\n", target->block->start->get()->line, target->block->start->get()->column, 
		target->block->end->get()->line, target->block->end->get()->column));

	for (auto _it = target->block->start; _it != target->block->end; ++_it) {
		LOG(_it->get()->string);
	}

	return { it, ctx.stack.num_evaluations + 1 };
}

std::optional<_operator> peek_unary_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, l_expression_context& context)
{
	if (it == end)
		return std::nullopt;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return std::nullopt;

	if (it->get()->is_punctuation() == false)
		return std::nullopt;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());
	
	if (ptr->is_operator(P_PAR_OPEN)) //parentheses are the operand, so go to next function
		return std::nullopt;

	if (bad_unary_operator(ptr->punc))
		throw linting_error(it->get(), "expected an expression instead of '%s'", ptr->string.c_str());

	if (!is_unary_operator(ptr->punc)) {
		return std::nullopt;
	}

	_operator v = { .priority = ptr->priority, .punc = ptr->punc };
	it = linting_data::getInstance().tokens->erase(it);
	return v;
}
bool peek_identifier(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, l_expression_context& context)
{
	if (it == end)
		return false;

	context.stack.assign_to_stack_if_possible(it); //should only be true when the current token is '('

	if (context.stack.time_to_exit()) {
		return true;
	}

	if (it->get()->is_punctuation()) {
		if (it->get()->is_operator(P_PAR_OPEN)) {

			std::advance(it, 1);
			expression_token_stack stack(P_PAR_OPEN, P_PAR_CLOSE);
			l_expression_results results = evaluate_expression_sanity(it, end, stack);
			if (results.num_evaluations > 1) {
				throw linting_error(results.it->get(), "expected one expression instead of %u", (unsigned __int64)results.num_evaluations);
			}
			
			it = results.it;

			context.stack.stack.num_close++;
			context.stack.location = it;

			std::advance(it, 1);
			return true;
		}
		return false;
	}
	
	auto& expression = context.expression;

	expression.identifier = it->get();

	//test if it's an identifier
	if (expression.identifier->tt == tokentype_t::IDENTIFIER) {

		//make sure it exists in the stack before it's used
		auto scope = linting_data::getInstance().active_scope;

		if (scope->variable_exists(expression.identifier->string) == false) {

			context.undefined_var = std::move(std::make_unique<undefined_variable>(
				undefined_variable{ .identifier = expression.identifier->string, .location = it, .num_args = 0 }));

			//throw linting_error(expression.identifier, "the identifier '%s' is undefined", expression.identifier->string.c_str());
		}
	}

	if (expression.identifier->is_reserved_keyword())
		throw linting_error(expression.identifier, "the '%s' keyword is a reserved keyword and it cannot be used here", expression.identifier->string.c_str());


	std::advance(it, 1);
	return true;
}
std::optional<_operator> peek_postfix_operator(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, l_expression_context& context)
{
	if (it == end)
		return std::nullopt;

	context.stack.assign_to_stack_if_possible(it);

	if (context.stack.time_to_exit())
		return std::nullopt;

	if (it->get()->is_punctuation() == false)
		return std::nullopt;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());
	
	if (!is_postfix_operator(ptr->punc))
		return std::nullopt;

	_operator v = { .priority = ptr->priority, .punc = ptr->punc };

	if (ptr->punc == P_BRACKET_OPEN) {
		it = evaluate_subscript_sanity(it, end, context);
	}
	else if (ptr->punc == P_PAR_OPEN) {
		it = evaluate_function_call_sanity(it, end, context);
	}
	else
		it = linting_data::getInstance().tokens->erase(it);

	return v;
}
std::list<singular>::iterator get_lowest_precedence(std::list<singular>::iterator& itr1, const std::list<singular>::iterator& end);

void generate_ast_tree_recursive(const std::unique_ptr<ast_node>& node, std::list<singular>& expressions)
{
	std::list<singular>::iterator itr1;

	if (expressions.size() == 1) {
		node->contents = std::make_unique<singular>(expressions.front());
		return;
	}

	itr1 = ++expressions.begin();
	itr1 = get_lowest_precedence(itr1, expressions.end());

	const auto& punctuation = *itr1;
	const auto& left_substr = itr1;
	const auto& right_substr = std::next(itr1);

	node->left = std::make_unique<ast_node>();
	node->right = std::make_unique<ast_node>();
	node->contents = std::make_unique<singular>(punctuation);

	//sort the left tree (everything to the left of the operator)
	std::list<singular> left_branch = std::list<singular>(expressions.begin(), left_substr);
	generate_ast_tree_recursive(node->left, left_branch);

	//sort the right tree (everything to the right of the operator)
	std::list<singular> right_branch = std::list<singular>(right_substr, expressions.end());
	generate_ast_tree_recursive(node->right, right_branch);

}

std::unique_ptr<ast_node> generate_ast_tree(std::list<singular>& expressions)
{
	nodeptr root = std::make_unique<ast_node>();
	generate_ast_tree_recursive(root, expressions);
	return std::move(root);

}
std::list<singular>::iterator get_lowest_precedence(std::list<singular>::iterator& itr1, const std::list<singular>::iterator& end)
{
	OperatorPriority priority{};
	OperatorPriority lowest_precedence = std::get<_operator>(itr1->value).priority;

	std::list<singular>::iterator lowest_precedence_i = itr1;

	if (itr1 == end)
		return lowest_precedence_i;

	do {
		const _operator& op = std::get<_operator>(itr1->value);
		priority = op.priority;
		
		if (priority < lowest_precedence) {
			lowest_precedence = priority;
			lowest_precedence_i = itr1;
		}

		std::advance(itr1, 2);

	} while (itr1 != end);

	return lowest_precedence_i;
}