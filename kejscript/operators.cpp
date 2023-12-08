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
std::unique_ptr<expression_node> evaluation_functions::arithmetic_addition(expression_node& left, expression_node& right)
{
	const auto& left_operand = std::get<std::unique_ptr<operand>>(left._op);
	const auto& right_operand = std::get<std::unique_ptr<operand>>(right._op);

	const auto left_value = left_operand->lvalue_to_rvalue();
	const auto right_value = right_operand->lvalue_to_rvalue();
	
	const integer_dt value = dynamic_cast<integer_dt*>(left_value)->get() + dynamic_cast<integer_dt*>(right_value)->get();

	LOG("result: " << value.get() << '\n');

	std::unique_ptr<datatype> v = std::make_unique<integer_dt>(value);
	auto oper = std::make_unique<operand>(std::move(v));
	return std::make_unique<expression_node>(oper);

}