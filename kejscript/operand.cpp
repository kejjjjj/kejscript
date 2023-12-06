#include "operand.hpp"
#include "runtime_exception.hpp"

operand::operand(const expression_t& expr)
{
	if (expr.prefix.size() || expr.postfix.size())
		throw runtime_error(expr.identifier, "not yet");

	if(expr.identifier->tt != tokentype_t::NUMBER_LITERAL)
		throw runtime_error(expr.identifier, "not yet");

	//throw runtime_error(expr.identifier, "ok\n");

	value = std::make_unique<integer_t>(std::stoi(expr.identifier->string));



}