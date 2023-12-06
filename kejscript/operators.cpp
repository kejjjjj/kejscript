#include "pch.hpp"
#include "operators.hpp"


void evaluation_functions::initialize_functions()
{
	static bool once = true;

	if (!once)
		return;

	eval_functions.insert({ P_ADD, arithmetic_addition });

	once = false;

}

void evaluation_functions::arithmetic_addition(expression_node& left, expression_node& right)
{
	LOG("result: " << dynamic_cast<integer_t*>(left._operand->value.get())->get() + dynamic_cast<integer_t*>(right._operand->value.get())->get() << '\n');

	LOG("pog\n");
}