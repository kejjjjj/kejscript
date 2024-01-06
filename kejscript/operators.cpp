#include "pch.hpp"
#include "operators.hpp"
#include "runtime_exception.hpp"

std::unordered_map<punctuation_e, evaluation_functions::funcptr>  evaluation_functions::eval_functions;

void evaluation_functions::initialize_functions()
{
	static bool once = true;

	if (!once)
		return;

	eval_functions.insert({ P_ADD, arithmetic_addition });
	eval_functions.insert({ P_SUB, arithmetic_subtraction });

	eval_functions.insert({ P_ASSIGN, assignment });
	eval_functions.insert({ P_LESS_THAN, less_than });
	eval_functions.insert({ P_MODULO, modulo });
	eval_functions.insert({ P_MULTIPLICATION, multiplication });

	eval_functions.insert({ P_EQUALITY, equality });

	once = false;

}
std::unique_ptr<operand> evaluation_functions::arithmetic_addition(operand& left_operand, operand& right_operand)
{

	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());

	auto& left_value = *left_operand.get_value();
	auto& right_value = *right_operand.get_value();
	
	switch (left_value.type()) {
	case datatype_e::bool_t:
		return create_rvalue<bool_dt>(left_value.getvalue<bool_dt>() + right_value.getvalue<bool_dt>());
	case datatype_e::int_t:
		return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() + right_value.getvalue<integer_dt>());
	case datatype_e::double_t:
		return create_rvalue<double_dt>(left_value.getvalue<double_dt>() + right_value.getvalue<double_dt>());
	default:
		throw runtime_error("an unexpected type");
	}

}
std::unique_ptr<operand> evaluation_functions::arithmetic_subtraction(operand& left_operand, operand& right_operand)
{
	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());

	auto& left_value = *left_operand.get_value();
	auto& right_value = *right_operand.get_value();

	switch (left_value.type()) {
	case datatype_e::bool_t:
		return create_rvalue<bool_dt>(left_value.getvalue<bool_dt>() - right_value.getvalue<bool_dt>());
	case datatype_e::int_t:
		return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() - right_value.getvalue<integer_dt>());
	case datatype_e::double_t:
		return create_rvalue<double_dt>(left_value.getvalue<double_dt>() - right_value.getvalue<double_dt>());
	default:
		throw runtime_error("an unexpected type");
	}

}
std::unique_ptr<operand> evaluation_functions::assignment(operand& left_operand, operand& right_operand)
{

	auto right_value = right_operand.lvalue_to_rvalue();

	if (left_operand.type != operand::Type::LVALUE)
		throw runtime_error(left_operand._operand, "cannot assign to a non-lvalue");
	
	auto& v = std::get<variable*>(left_operand.value);

	switch (right_value->type()) {
	case datatype_e::bool_t:
		v->value = datatype::cast<bool_dt>(right_value);
		break;
	case datatype_e::int_t:
		v->value = datatype::cast<integer_dt>(right_value);
		break;
	case datatype_e::double_t:
		v->value = datatype::cast<double_dt>(right_value);
		break;
	}
	v->initialized = true;

	return create_lvalue(v);
}
std::unique_ptr<operand> evaluation_functions::equality(operand& left_operand, operand& right_operand)
{

	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());

	auto& left_value = *left_operand.get_value();
	auto& right_value = *right_operand.get_value();

	switch (left_value.type()) {
	case datatype_e::bool_t:
		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
			(datatype::cast_normal<bool_dt>(&left_value) == datatype::cast_normal<bool_dt>(&right_value)));

	case datatype_e::int_t:
		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
	(datatype::cast_normal<integer_dt>(&left_value) == datatype::cast_normal<integer_dt>(&right_value)));	
	
	case datatype_e::double_t:
		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
			(datatype::cast_normal<double_dt>(&left_value) == datatype::cast_normal<double_dt>(&right_value)));

	}

	return nullptr;
}
std::unique_ptr<operand> evaluation_functions::less_than(operand& left_operand, operand& right_operand)
{

	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());

	auto& left_value = *left_operand.get_value();
	auto& right_value = *right_operand.get_value();

	switch (left_value.type()) {
	case datatype_e::bool_t:
		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
			(datatype::cast_normal<bool_dt>(&left_value) < datatype::cast_normal<bool_dt>(&right_value)));

	case datatype_e::int_t:
		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
			(datatype::cast_normal<integer_dt>(&left_value) < datatype::cast_normal<integer_dt>(&right_value)));

	case datatype_e::double_t:
		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
			(datatype::cast_normal<double_dt>(&left_value) < datatype::cast_normal<double_dt>(&right_value)));
	}

	return nullptr;
}
std::unique_ptr<operand> evaluation_functions::modulo(operand& left_operand, operand& right_operand)
{
	if (left_operand.is_integral() == false) {
		throw runtime_error(left_operand._operand, "the operand must have an integral type");
	}else if (right_operand.is_integral() == false) {
		throw runtime_error(right_operand._operand, "the operand must have an integral type");
	}

	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());

	auto& left_value = *left_operand.get_value();
	auto& right_value = *right_operand.get_value();

	//both should have integer types now...
	return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() % right_value.getvalue<integer_dt>());
}
std::unique_ptr<operand> evaluation_functions::multiplication(operand& left_operand, operand& right_operand)
{
	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());

	auto& left_value = *left_operand.get_value();
	auto& right_value = *right_operand.get_value();

	switch (left_value.type()) {
	case datatype_e::bool_t:
		return create_rvalue<bool_dt>(left_value.getvalue<bool_dt>() * right_value.getvalue<bool_dt>());
	case datatype_e::int_t:
		return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() * right_value.getvalue<integer_dt>());
	case datatype_e::double_t:
		return create_rvalue<double_dt>(left_value.getvalue<double_dt>() * right_value.getvalue<double_dt>());
	default:
		throw runtime_error("an unexpected type");
	}
}
void evaluation_functions::assign_to_lvalue(variable* var, operand& right)
{

	auto right_operand = right.lvalue_to_rvalue();
	auto& left_operand = var->value;

	switch (right_operand->type()) {
	case datatype_e::bool_t:
		left_operand = datatype::cast<bool_dt>(right_operand);
		break;
	case datatype_e::int_t:
		left_operand = datatype::cast<integer_dt>(right_operand);
		break;
	case datatype_e::double_t:
		left_operand = datatype::cast<double_dt>(right_operand);
		break;
	}
	var->initialized = true;
	return;
}