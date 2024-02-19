#include "operand.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"
#include "operators.hpp"
#include "runtime_expression.hpp"

operand::operand(singular& expr, function_stack* stack) : value(), _operand(expr.token)
{
	using rvalue = std::unique_ptr<datatype>;


	//static_assert(std::variant_size_v<decltype(value)> == 2, "No alternatives in the variant");

	auto& oper = expr.v;

	if (oper.type == validation_expression::Type::OTHER) {
		const auto& table = stack->variables;

		auto& r = table[ std::get<validation_expression::other>(oper.value).variable_index ];
		
		value = r;
		type = Type::LVALUE;

		return;
	}
	else {
		type = Type::RVALUE;
		auto& literal = std::get<validation_expression::literal>(oper.value);
		using literalType = validation_expression::literal;
		switch (literal.type) {
		case literalType::NUMBER_LITERAL:
			value = std::make_unique<integer_dt>(*reinterpret_cast<int32_t*>(literal.value.data()));
			break;
		case literalType::FLOAT_LITERAL:
			value = std::make_unique<double_dt>(*reinterpret_cast<double*>(literal.value.data()));
			break;
		case literalType::STRING_LITERAL:
			value = std::make_unique<string_dt>(literal.value);

			string = std::make_unique<string_object>();

			for (auto& v : literal.value)
				string->insert(v);

			break;
		case literalType::CHAR_LITERAL:
			value = std::make_unique<char_dt>(literal.value[0]);
			break;
		case literalType::_TRUE:
		case literalType::_FALSE:
			value = std::make_unique<bool_dt>(literal.type == literalType::_TRUE ? true : false);
			break;
		default:
			throw runtime_error(expr.token, "huh?");
		}
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

	auto& weaker_value = std::get<rvalue>(weaker->value);
	auto& v = *weaker_value;


	//bool_t is not needed here because it will never be stronger than anything
	switch (stronger->get_value()->type()) {
	case datatype_e::char_t:
		weaker_value = datatype::create_type_ptr<char_dt, char>(v);
		break;
	case datatype_e::int_t:
		weaker_value = datatype::create_type_ptr<integer_dt, int>(v);
		break;
	case datatype_e::double_t:
		weaker_value = datatype::create_type_ptr<double_dt, double>(v);
		break;
	}

}
void operand::cast_weaker_operand(datatype* other)
{
	datatype* v = get_value();

	datatype_e this_type = v->type();
	datatype_e other_type = other->type();

	datatype* stronger = 0;
	datatype* weaker = 0;

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

	switch (stronger->type()) {
	case datatype_e::int_t:
		*weaker = datatype::create_type_copy<integer_dt, int>(datatype::cast_normal<integer_dt>(weaker));
		break;
	case datatype_e::double_t:
		*weaker = datatype::create_type_copy<double_dt, double>(datatype::cast_normal<double_dt>(weaker));
		break;
	}

}
void operand::implicit_cast(operand& other)
{
	datatype* l = get_value();
	datatype* r = other.get_value();

	if (l->type() == r->type())
		return;

	cast_weaker_operand(l->type(), r->type(), other);

	return;

}
void operand::implicit_cast(datatype* other)
{
	datatype* l = get_value();
	datatype* r = other;

	if (l->type() == r->type())
		return;

	cast_weaker_operand(other);
	return;

}
void operand::implicit_cast(operand& other, datatype* l, datatype* r)
{
	if (l->type() == r->type())
		return;

	if(type == Type::RVALUE)
		return cast_weaker_operand(l->type(), r->type(), other);



	return;
}
bool operand::has_value()  const noexcept
{
	return is_object() || is_string() || get_value();
}
bool operand::is_integral() const noexcept
{
	return !is_object() && get_value()->is_integral();
}
bool operand::is_numeric() const noexcept
{
	return !is_object() && get_value()->is_numeric();
}
bool operand::bool_convertible() const noexcept
{
	return !is_object() && get_value()->bool_convertible();
}
[[maybe_unused]] datatype* operand::lvalue_to_rvalue()
{
	if (type == Type::RVALUE)
		return std::get<rvalue>(value).get();

	auto& lval = std::get<std::shared_ptr<variable>>(value);

	if (lval->initialized == false)
		throw runtime_error(_operand, "use of an uninitialized variable '%s'", lval->identifier.c_str());

	if (auto str = is_string()) {

		auto _string = str->get_string();

		string = std::make_unique<string_object>();

		for (auto& s : _string)
			string->insert(s);

		value = std::make_unique<string_dt>(str->get_string());
		type = Type::RVALUE;

		return std::get<rvalue>(value).get();
	}

	auto dtype = (lval->value).get();


	switch (dtype->type()) {
	case datatype_e::bool_t:
		value = datatype::cast<bool_dt>(dtype);
		break;
	case datatype_e::int_t:
		value = datatype::cast<integer_dt>(dtype);
		break;
	case datatype_e::double_t:
		value = datatype::cast<double_dt>(dtype);
		break;
	case datatype_e::string_t:
		value = datatype::cast<string_dt>(dtype);
		break;
	case datatype_e::char_t:
		value = datatype::cast<char_dt>(dtype);
		break;
	}

	type = Type::RVALUE;

	return std::get<rvalue>(value).get();
}
datatype* operand::get_value()
{
	if (type == Type::RVALUE)
		return std::get<rvalue>(value).get();

	auto& v = std::get<std::shared_ptr<variable>>(value);

	return type == Type::RVALUE ? std::get<rvalue>(value).get() : (v->value).get();
}
datatype* operand::get_value() const
{
	if (type == Type::RVALUE)
		return std::get<rvalue>(value).get();

	auto& v = std::get<std::shared_ptr<variable>>(value);

	return type == Type::RVALUE ? std::get<rvalue>(value).get() : (v->value).get();
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