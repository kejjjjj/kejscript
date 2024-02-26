#include "pch.hpp"
#include "operators.hpp"
#include "runtime_exception.hpp"

std::unordered_map<punctuation_e, evaluation_functions::funcptr>  evaluation_functions::eval_functions;
std::unordered_map<punctuation_e, evaluation_functions::unaryfuncptr> evaluation_functions::unary_functions;

void evaluation_functions::initialize_functions()
{
	static bool once = true;

	if (!once)
		return;

	eval_functions.insert({ P_ADD, arithmetic_addition });
	//eval_functions.insert({ P_SUB, arithmetic_subtraction });

	eval_functions.insert({ P_ASSIGN, assignment });
	eval_functions.insert({ P_LESS_THAN, less_than });
	//eval_functions.insert({ P_GREATER_THAN, greater_than });

	//eval_functions.insert({ P_MODULO, modulo });
	//eval_functions.insert({ P_MULTIPLICATION, multiplication });
	//eval_functions.insert({ P_DIVISION, division});

	//eval_functions.insert({ P_EQUALITY, equality });
	//eval_functions.insert({ P_UNEQUALITY, unequality });

	//eval_functions.insert({ P_LOGICAL_AND, logical_and });

	unary_functions.insert({ P_INCREMENT, increment });
	//unary_functions.insert({ P_SUB, negate });

	once = false;

}
operand* evaluation_functions::arithmetic_addition(operand* left_operand, operand* right_operand)
{


	left_operand->implicit_cast(*right_operand, &left_operand->lvalue_to_rvalue(), &right_operand->lvalue_to_rvalue());

	auto& left_value = *left_operand->get_value();
	auto& right_value = *right_operand->get_value();
	
	switch (left_value.rvalue_t) {
	case datatype_e::bool_t:
		left_operand->make_rvalue(left_value.get<bool_dt>() + right_value.get<bool_dt>());
		break;
	case datatype_e::int_t:
		left_operand->make_rvalue(left_value.get<integer_dt>() + right_value.get<integer_dt>());
		break;
	case datatype_e::double_t:
		left_operand->make_rvalue(left_value.get<double_dt>() + right_value.get<double_dt>());
		break;
	case datatype_e::char_t:
		left_operand->make_rvalue(left_value.get<char_dt>() + right_value.get<char_dt>());
		break;
	default:
		throw runtime_error(left_operand->_operand, "this operation for the type '%s' is unsupported", left_operand->get_type().c_str());
	}

	return left_operand;
}
//std::unique_ptr<operand> evaluation_functions::arithmetic_subtraction(operand& left_operand, operand& right_operand)
//{
//	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	switch (left_value.type()) {
//	case datatype_e::bool_t:
//		return create_rvalue<bool_dt>(left_value.getvalue<bool_dt>() - right_value.getvalue<bool_dt>());
//	case datatype_e::int_t:
//		return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() - right_value.getvalue<integer_dt>());
//	case datatype_e::double_t:
//		return create_rvalue<double_dt>(left_value.getvalue<double_dt>() - right_value.getvalue<double_dt>());
//	case datatype_e::char_t:
//		return create_rvalue<char_dt>(left_value.getvalue<char_dt>() - right_value.getvalue<char_dt>());
//	default:
//		throw runtime_error(left_operand._operand, "this operation for the type '%s' is unsupported", left_operand.get_type().c_str());
//
//	}
//
//}
operand* evaluation_functions::assignment(operand* left_operand, operand* right_operand)
{

	if (left_operand->type != operand::Type::LVALUE)
		throw runtime_error(left_operand->_operand, "cannot assign to a non-lvalue");
	
	auto& v = (left_operand->value);

	if(v->immutable)
		throw runtime_error(left_operand->_operand, "the operand is immutable");

	assign_to_lvalue(v, right_operand);
	return left_operand;
}
void evaluation_functions::assign_to_lvalue(std::shared_ptr<variable>& var, const operand* right)
{
	return assign_to_lvalue(var, right);
}
void evaluation_functions::assign_to_lvalue(std::shared_ptr<variable>& var, operand* right)
{
	if (auto func_ptr = right->is_function_pointer()) {
		var->function_pointer = func_ptr;
		var->string = 0;
		var->obj = 0;
		var->initialized = true;
		return;
	}
	if (auto obj = right->is_object()) {

		var->function_pointer = 0;
		var->obj = obj;
		var->initialized = true;
		//var->function_pointer = right->is_function_pointer();

		return;
	}
	if (auto string = right->is_string()) {

		var->function_pointer = 0;
		var->string = std::make_shared<string_object>();
		auto str = string->get_string();
		for (auto& c : str)
			var->string->insert(c);

		var->initialized = true;
		return;
	}
	if (right->has_value() == false) {
		throw runtime_error(right->_operand, "the operand does not have a value");
	}

	var->string = 0;
	var->obj = 0;
	var->function_pointer = 0;

	auto& right_operand = right->lvalue_to_rvalue();
	auto& left_operand = var->get_value();

	left_operand = right_operand;

	//switch (right_operand.rvalue_t) {
	//case datatype_e::bool_t:
	//	
	//	left_operand = datatype::cast<bool_dt>(right_operand);
	//	break;
	//case datatype_e::int_t:
	//	left_operand = datatype::cast<integer_dt>(right_operand);
	//	break;
	//case datatype_e::double_t:
	//	left_operand = datatype::cast<double_dt>(right_operand);
	//	break;
	//case datatype_e::string_t:
	//	left_operand = datatype::cast<string_dt>(right_operand);
	//	break;
	//case datatype_e::char_t:
	//	left_operand = datatype::cast<char_dt>(right_operand);
	//	break;
	//}

	var->initialized = true;
	return;
}
//std::unique_ptr<operand> evaluation_functions::equality(operand& left_operand, operand& right_operand)
//{
//	//if the types aren't compatible then return false
//	if (left_operand.is_compatible_with(right_operand) == false) {
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>(bool_dt(false)));
//	}
//
//	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	switch (left_value.type()) {
//	case datatype_e::bool_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<bool_dt>(&left_value) == datatype::cast_normal<bool_dt>(&right_value)));
//
//	case datatype_e::int_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//	(datatype::cast_normal<integer_dt>(&left_value) == datatype::cast_normal<integer_dt>(&right_value)));	
//	
//	case datatype_e::double_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<double_dt>(&left_value) == datatype::cast_normal<double_dt>(&right_value)));
//
//	case datatype_e::char_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<char_dt>(&left_value) == datatype::cast_normal<char_dt>(&right_value)));
//
//	case datatype_e::string_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(bool_dt(left_operand.is_string()->get_string() == right_operand.is_string()->get_string())));
//	}
//
//	throw runtime_error(left_operand._operand, "this operation for the type '%s' is unsupported", left_operand.get_type().c_str());
//
//
//	return nullptr;
//}
//std::unique_ptr<operand> evaluation_functions::unequality(operand& left_operand, operand& right_operand)
//{
//	//if the types aren't compatible then return true
//	if (left_operand.is_compatible_with(right_operand) == false) {
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>(bool_dt(true)));
//	}
//
//	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	switch (left_value.type()) {
//	case datatype_e::bool_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<bool_dt>(&left_value) != datatype::cast_normal<bool_dt>(&right_value)));
//
//	case datatype_e::int_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<integer_dt>(&left_value) != datatype::cast_normal<integer_dt>(&right_value)));
//
//	case datatype_e::double_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<double_dt>(&left_value) != datatype::cast_normal<double_dt>(&right_value)));
//
//	case datatype_e::char_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<char_dt>(&left_value) != datatype::cast_normal<char_dt>(&right_value)));
//	case datatype_e::string_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(bool_dt(left_operand.is_string()->get_string() != right_operand.is_string()->get_string())));
//	}
//
//	throw runtime_error(left_operand._operand, "this operation for the type '%s' is unsupported", left_operand.get_type().c_str());
//
//
//	return nullptr;
//}
//std::unique_ptr<operand> evaluation_functions::logical_and(operand& left_operand, operand& right_operand)
//{
//	if (left_operand.bool_convertible() == false)
//		throw runtime_error(left_operand._operand, "the operand must be convertible to a boolean");
//	if (right_operand.bool_convertible() == false)
//		throw runtime_error(right_operand._operand, "the operand must be convertible to a boolean");
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	bool left_oper = *reinterpret_cast<bool*>(left_value.value.data());
//	bool right_oper = *reinterpret_cast<bool*>(right_value.value.data());
//
//	return create_rvalue<bool_dt>(bool_dt(left_oper && right_oper));
//}
operand* evaluation_functions::less_than(operand* left_operand, operand* right_operand)
{

	left_operand->implicit_cast(*right_operand, &left_operand->lvalue_to_rvalue(), &right_operand->lvalue_to_rvalue());

	auto& left_value = *left_operand->get_value();
	auto& right_value = *right_operand->get_value();

	switch (left_value.rvalue_t) {
	case datatype_e::bool_t:
		left_operand->make_rvalue(left_value.get<bool_dt>() < right_value.get<bool_dt>());
		break;
	case datatype_e::int_t:
		left_operand->make_rvalue(left_value.get<integer_dt>() < right_value.get<integer_dt>());
		break;
	case datatype_e::double_t:
		left_operand->make_rvalue(left_value.get<double_dt>() < right_value.get<double_dt>());
		break;
	case datatype_e::char_t:
		left_operand->make_rvalue(left_value.get<char_dt>() < right_value.get<char_dt>());
		break;
	default:
		throw runtime_error(left_operand->_operand, "this operation for the type '%s' is unsupported", left_operand->get_type().c_str());
	}

	return left_operand;
}
//std::unique_ptr<operand> evaluation_functions::greater_than(operand& left_operand, operand& right_operand)
//{
//
//	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	switch (left_value.type()) {
//	case datatype_e::bool_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<bool_dt>(&left_value) > datatype::cast_normal<bool_dt>(&right_value)));
//
//	case datatype_e::int_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<integer_dt>(&left_value) > datatype::cast_normal<integer_dt>(&right_value)));
//
//	case datatype_e::double_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<double_dt>(&left_value) > datatype::cast_normal<double_dt>(&right_value)));
//
//	case datatype_e::char_t:
//		return create_rvalue<bool_dt>(datatype::create_type<bool_dt, bool>
//			(datatype::cast_normal<char_dt>(&left_value) > datatype::cast_normal<char_dt>(&right_value)));
//	}
//
//	throw runtime_error(left_operand._operand, "this operation for the type '%s' is unsupported", left_operand.get_type().c_str());
//
//
//	return nullptr;
//}
//std::unique_ptr<operand> evaluation_functions::modulo(operand& left_operand, operand& right_operand)
//{
//	if (left_operand.is_integral() == false) {
//		throw runtime_error(left_operand._operand, "the operand must have an integral type");
//	}else if (right_operand.is_integral() == false) {
//		throw runtime_error(right_operand._operand, "the operand must have an integral type");
//	}
//
//	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	auto type = left_value.type();
//
//	if (type == datatype_e::int_t) {
//
//		auto& right = right_value.getvalue<integer_dt>();
//
//		if (right.get() == 0)
//			throw runtime_error(right_operand._operand, "division by zero");
//
//		//both should have integer types now...
//		return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() % right_value.getvalue<integer_dt>());
//	}
//	else if (type == datatype_e::char_t) {
//
//		auto& right = right_value.getvalue<char_dt>();
//
//		if (right.get() == 0)
//			throw runtime_error(right_operand._operand, "division by zero");
//
//		//both should have integer types now...
//		return create_rvalue<char_dt>(left_value.getvalue<char_dt>() % right_value.getvalue<char_dt>());
//	}
//
//	return nullptr;
//}
//std::unique_ptr<operand> evaluation_functions::multiplication(operand& left_operand, operand& right_operand)
//{
//	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	switch (left_value.type()) {
//	case datatype_e::bool_t:
//		return create_rvalue<bool_dt>(left_value.getvalue<bool_dt>() * right_value.getvalue<bool_dt>());
//	case datatype_e::int_t:
//		return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() * right_value.getvalue<integer_dt>());
//	case datatype_e::double_t:
//		return create_rvalue<double_dt>(left_value.getvalue<double_dt>() * right_value.getvalue<double_dt>());
//	case datatype_e::char_t:
//		return create_rvalue<char_dt>(left_value.getvalue<char_dt>() * right_value.getvalue<char_dt>());
//	default:
//		throw runtime_error(left_operand._operand, "this operation for the type '%s' is unsupported", left_operand.get_type().c_str());
//
//	}
//}
//std::unique_ptr<operand> evaluation_functions::division(operand& left_operand, operand& right_operand)
//{
//	left_operand.implicit_cast(right_operand, left_operand.lvalue_to_rvalue(), right_operand.lvalue_to_rvalue());
//
//	auto& left_value = *left_operand.get_value();
//	auto& right_value = *right_operand.get_value();
//
//	integer_dt* iright = 0;
//	char_dt* cright = 0;
//
//	switch (left_value.type()) {
//	case datatype_e::bool_t:
//		throw runtime_error(left_operand._operand, "don't divide booleans");
//	case datatype_e::double_t:
//		return create_rvalue<double_dt>(left_value.getvalue<double_dt>() / right_value.getvalue<double_dt>());
//	case datatype_e::int_t:
//		iright = &right_value.getvalue<integer_dt>();
//
//		if (iright->get() == 0)
//			throw runtime_error(right_operand._operand, "division by zero");
//
//		return create_rvalue<integer_dt>(left_value.getvalue<integer_dt>() / *iright);
//	case datatype_e::char_t:
//		cright = &right_value.getvalue<char_dt>();
//
//		if (cright->get() == 0)
//			throw runtime_error(right_operand._operand, "division by zero");
//
//		return create_rvalue<char_dt>(left_value.getvalue<char_dt>() / *cright);
//
//	}
//	throw runtime_error(left_operand._operand, "this operation for the type '%s' is unsupported", left_operand.get_type().c_str());
//
//}











//UNARY
operand* evaluation_functions::increment(operand* op)
{

	if (op->type != operand::Type::LVALUE)
		throw runtime_error(op->_operand, "cannot increment a non-lvalue");

	if (op->value->initialized == false) {
		throw runtime_error(op->_operand, "cannot increment an uninitialized variable");
	}

	if (op->is_integral() == false)
		throw runtime_error(op->_operand, "incrementing non-integral types is not allowed");

	auto value = op->get_value();


	switch (value->rvalue_t) {
	case datatype_e::bool_t:
		throw runtime_error(op->_operand, "incrementing a bool value is not allowed");
	case datatype_e::int_t:
		++(*reinterpret_cast<std::int64_t*>(&value->get<integer_dt>().value));
		break;
	case datatype_e::double_t:
		++(*reinterpret_cast<double*>(&value->get<double_dt>().value));
		break;
	case datatype_e::char_t:
		++(value->get<double_dt>().value[0]);
		break;
	default:
		throw runtime_error(op->_operand, "the type '%s' cannot be incremented", op->get_type().c_str());
	}

	return std::move(op);
}
//operand_ptr evaluation_functions::negate(operand_ptr& op)
//{
//
//	if (op->type == operand::Type::LVALUE && std::get<std::shared_ptr<variable>>(op->value)->initialized == false) {
//		throw runtime_error(op->_operand, "cannot increment an uninitialized variable");
//	}
//
//	if (op->is_numeric() == false)
//		throw runtime_error(op->_operand, "negating non-numeric types is not allowed");
//
//	auto value = op->get_value();
//
//	switch (value->type()) {
//	case datatype_e::bool_t:
//		throw runtime_error(op->_operand, "negating a bool value is not allowed");
//	case datatype_e::int_t:
//		return create_rvalue<integer_dt>(-value->getvalue<integer_dt>());
//	case datatype_e::double_t:
//		return create_rvalue<double_dt>(-value->getvalue<double_dt>());
//	case datatype_e::char_t:
//		return create_rvalue<char_dt>(-value->getvalue<char_dt>());
//	default:
//		throw runtime_error(op->_operand, "the type '%s' cannot be negated", op->get_type().c_str());
//	}
//
//	return nullptr;
//}

operand* subscript(operand* op, operand* expression)
{


	if (expression->is_integral() == false)
		throw runtime_error(op->_operand, "the [] index must be integral");

	auto index = *reinterpret_cast<std::int64_t*>(expression->get_value()->data->data());

	auto obj = op->is_object();
	auto string = op->is_string();
	std::int64_t size = 0;

	operand_ptr result;

	if (!obj && !string)
		throw runtime_error(op->_operand, "the [] operator can only be used on strings and arrays");

	if (string) {
		size = static_cast<std::int64_t>(string->variables.size());

		if (index < 0 || index >= size) {
			throw runtime_error(op->_operand, "the index '%s' is outside of the array bounds", std::to_string(index).c_str());
		}

		op->value = string->variables[index];
		op->type = operand::Type::LVALUE;

		return op;
	}
	else {
		size = static_cast<std::int64_t>(obj->variables.size());

		if (index < 0 || index >= size) {
			throw runtime_error(op->_operand, "the index '%s' is outside of the array bounds", std::to_string(index).c_str());
		}

		op->value = string->variables[index];
		op->type = operand::Type::LVALUE;

		return op;

	}

	return nullptr;
}