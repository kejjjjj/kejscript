#include "pch.hpp"

struct compiler_expression
{
	std::vector<token_t*> prefix;
	token_t* identifier = 0;
	std::vector<token_t*> postfix;
	bool op = false;
};

bool peek_unary_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end);
bool peek_identifier(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end);
bool peek_postfix_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end);

std::vector<compiler_expression> expressions;
compiler_expression expression;

void tokenize_operand(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	while (peek_unary_operator(it, end));
	if (peek_identifier(it, end) == false)
		throw compile_error(it->get(), "expected an identifier instead of '%s'", it->get()->string.c_str());
	while (peek_postfix_operator(it, end));

	expression.op = false;
	expressions.push_back(expression);
	expression = compiler_expression();

}
void tokenize_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	if (it == end)
		return;

	const auto token = it->get();

	if(token->is_punctuation() == false)
		throw compile_error(token, "expected an operator or ';' instead of %s", token->string.c_str());

	auto op = dynamic_cast<punctuation_token_t*>(token);

	if (!satisfies_operator(op->punc)) {
		throw compile_error(token, "expected an operator or ';' instead of %s", token->string.c_str());

	}

	if(op->punc == punctuation_e::P_ASSIGN)
		throw compile_error(token, "don't use assignment operators in expressions");


	expression.op = true;
	expression.identifier = token;
	expressions.push_back(expression);
	expression = compiler_expression();
	std::advance(it, 1);
	return;

}
void evaluate_expression_sanity([[maybe_unused]] VectorTokenPtr::iterator& it, [[maybe_unused]] VectorTokenPtr::iterator& end)
{
	if (it == end)
		throw compile_error(it->get(), "an empty expression is not allowed");

	expressions.clear();
	while (it != end) {
		tokenize_operand(it, end);
		tokenize_operator(it, end);

	}

	LOG("there are " << expressions.size() << " parts to this expression\n");

}

bool peek_unary_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	if (it == end)
		return false;

	if (it->get()->is_punctuation() == false)
		return false;
	
	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());

	if (bad_unary_operator(ptr->punc))
		throw compile_error("'%s' is not a valid unary operator", ptr->string.c_str());

	if (!is_unary_operator(ptr->punc)) {
		return false;
	}
	expression.prefix.push_back(it->get());
	std::advance(it, 1);
	return true;
}
bool peek_identifier(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	if (it == end)
		return false;

	if (it->get()->is_punctuation())
		return false;
	expression.identifier = it->get();

	//test if it's an identifier
	if (expression.identifier->tt == tokentype_t::IDENTIFIER) {

		//make sure it exists in the stack before it's used
		auto scope = compiler_data::getInstance().active_scope;

		if (scope->variable_exists(expression.identifier->string) == false) {
			throw compile_error(expression.identifier, "the identifier '%s' is undefined", expression.identifier->string.c_str());
		}
	}

	std::advance(it, 1);
	return true;
}
bool peek_postfix_operator(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	if (it == end)
		return false;

	if (it->get()->is_punctuation() == false)
		return false;

	auto ptr = dynamic_cast<punctuation_token_t*>(it->get());
	
	if (!is_postfix_operator(ptr->punc))
		return false;

	expression.postfix.push_back(it->get());
	std::advance(it, 1);
	return true;
}