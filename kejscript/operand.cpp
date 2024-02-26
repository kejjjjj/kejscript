#include "operand.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"
#include "operators.hpp"
#include "runtime_expression.hpp"

operand::operand(singular& expr, function_stack* stack) : value(), _operand(expr.token)
{
	auto& oper = expr.v;

	if (oper.type == validation_expression::Type::OTHER) {

		std::vector<std::shared_ptr<variable>>* ref = 0;

		auto& v = std::get<validation_expression::other>(oper.value);

		if (expr.structure && stack->_this)
			ref = &stack->_this->variables;
		
		auto& r = ref ? (*ref)[ v.variable_index ] : stack->variables[v.operand_index]->get_lvalue();
		
		if (expr.function_pointer) {
			r->function_pointer = expr.function_pointer;
			r->immutable = true;
		}

		make_lvalue(r);
		return;
	}
	else {
		type = Type::RVALUE;
		rvalue = *runtime::sorted_literals[std::get<validation_expression::literal>(oper.value).literal_index]->value.get();

	}
	

	//todo: evaluate postfix and prefix



}
void operand::cast_weaker_operand(datatype_e this_type, datatype_e other_type, operand& other)
{
	operand* stronger = nullptr;
	operand* weaker = nullptr;

	if (this_type > other_type) {
		stronger = this;
		weaker = &other;
	}
	else if (this_type < other_type) {
		stronger = &other;
		weaker = this;
	}
	else
		return;

	//auto& weaker_value = (weaker->rvalue);
	//auto& v = weaker_value;

	*weaker->get_value() = *stronger->get_value();

	//bool_t is not needed here because it will never be stronger than anything
	//switch (stronger->get_value()->type()) {
	//case datatype_e::char_t:
	//	*weaker_value = datatype::cast_normal<char_dt>(v);
	//	break;
	//case datatype_e::int_t:
	//	*weaker_value = datatype::cast_normal<integer_dt>(v);
	//	break;
	//case datatype_e::double_t:
	//	*weaker_value = datatype::cast_normal<double_dt>(v);
	//	break;
	//}

}
void operand::cast_weaker_operand(type_value* other)
{
	type_value* v = get_value();

	datatype_e this_type = v->rvalue_t;
	datatype_e other_type = other->rvalue_t;

	type_value* stronger = 0;
	type_value* weaker = 0;

	if (this_type > other_type) {
		stronger = v;
		weaker = other;
	}
	else if (this_type < other_type) {
		stronger = other;
		weaker = v;
	}
	else
		return;

	weaker = stronger;

	//switch (stronger->type()) {
	//case datatype_e::int_t:
	//	*weaker = datatype::cast_normal<integer_dt>(weaker);
	//	break;
	//case datatype_e::double_t:
	//	*weaker = datatype::cast_normal<double_dt>(weaker);
	//	break;
	//}

}
void operand::implicit_cast(operand& other)
{
	auto l = get_value();
	auto r = other.get_value();

	if (l->rvalue_t == r->rvalue_t)
		return;

	cast_weaker_operand(l->rvalue_t, r->rvalue_t, other);
	return;

}
void operand::implicit_cast(type_value* r)
{
	auto l = get_value();

	if (l->rvalue_t == r->rvalue_t)
		return;

	cast_weaker_operand(r);
	return;

}
void operand::implicit_cast(operand& other, type_value* l, type_value* r)
{
	if (l->rvalue_t == r->rvalue_t)
		return;

	if(type == Type::RVALUE)
		return cast_weaker_operand(l->rvalue_t, r->rvalue_t, other);

}
bool operand::has_value()  const noexcept
{
	return is_object() || is_function_pointer() || is_string() || get_value();
}
bool operand::is_integral() const noexcept
{
	return !is_object() && !is_function_pointer() && !is_string() && get_value()->is_integral();
}
bool operand::is_numeric() const noexcept
{
	return !is_object() && !is_function_pointer() && !is_string() && get_value()->is_numeric();
}
bool operand::bool_convertible() const noexcept
{
	return !is_object() && !is_function_pointer() && !is_string() && get_value()->bool_convertible();
}
bool operand::valueless_type() const noexcept
{
	return this && (is_object() || is_function_pointer());
}
[[maybe_unused]] type_value& operand::lvalue_to_rvalue()
{
	if (type == Type::RVALUE)
		return rvalue;

	auto& lval = value;
	
	if(valueless_type())
		throw runtime_error(_operand, "cannot make the type '%s' into an rvalue", get_type().c_str());

	if (lval->initialized == false)
		throw runtime_error(_operand, "use of an uninitialized variable '%s'", lval->identifier.c_str());

	if (auto str = is_string()) {

		auto _string = str->get_string();

		string = std::make_unique<string_object>();

		for (auto& s : _string)
			string->insert(s);

		make_rvalue<string_dt>(string_dt(str->get_string()));

		return rvalue;
	}

	auto& dtype = (lval->get_value());

	rvalue = dtype;
	type = Type::RVALUE;

	//make_rvalue(dtype);

	//switch (lval->value_t) {
	//case datatype_e::bool_t:
	//	make_rvalue(datatype::cast_normal<bool_dt>(dtype));
	//	break;
	//case datatype_e::int_t:
	//	make_rvalue(datatype::cast_normal<integer_dt>(dtype));
	//	break;
	//case datatype_e::double_t:
	//	make_rvalue(datatype::cast_normal<double_dt>(dtype));
	//	break;
	//case datatype_e::string_t:
	//	make_rvalue(datatype::cast_normal<string_dt>(dtype));
	//	break;
	//case datatype_e::char_t:
	//	make_rvalue(datatype::cast_normal<char_dt>(dtype));
	//	break;
	//}

	return rvalue;
}
type_value* operand::get_value()
{
	if (type == Type::RVALUE)
		return &rvalue;

	return &value->get_value();
}
type_value* operand::get_value() const
{
	if (type == Type::RVALUE)
		return const_cast<type_value*>(&rvalue);

	return &value->get_value();
}
bool operand::is_compatible_with(const operand& other) const
{
	if ((is_object() == nullptr) != (other.is_object() == nullptr)) {
		return false;
	}

	if ((is_string() == nullptr) != (other.is_string() == nullptr)) {
		return false;
	}

	return true;
}