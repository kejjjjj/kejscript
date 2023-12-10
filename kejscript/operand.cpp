#include "operand.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

operand::operand(const expression_t& expr) : value(), _operand(expr.identifier)
{
	using rvalue = std::unique_ptr<datatype>;


	static_assert(std::variant_size_v<decltype(value)> == 2, "No alternatives in the variant");


	if (expr.identifier->is_identifier()) {
		const auto& table = runtime::get_instance().stack->variables;
		variable* r = table.find(expr.identifier->string)->second.get();
		value = r;
		type = Type::LVALUE;

		return;
	}
	else {
		type = Type::RVALUE;
		switch (expr.identifier->tt) {
		case tokentype_t::NUMBER_LITERAL:
			value = std::make_unique<integer_dt>(std::stoi(expr.identifier->string));
			break;
		case tokentype_t::FLOAT_LITERAL:
			value = std::make_unique<double_dt>(std::stod(expr.identifier->string));
			break;
		case tokentype_t::_TRUE:
		case tokentype_t::_FALSE:
			value = std::make_unique<bool_dt>(expr.identifier->tt == tokentype_t::_TRUE ? true : false);
			break;
		default:
			throw runtime_error(expr.identifier, "huh?");
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
	case datatype_e::int_t:
		weaker_value = datatype::create_type_ptr<integer_dt, int>(v);
		break;
	case datatype_e::double_t:
		weaker_value = datatype::create_type_ptr<double_dt, float>(v);
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
void operand::implicit_cast(operand& other, datatype* l, datatype* r)
{
	if (l->type() == r->type())
		return;

	if(type == Type::RVALUE)
		return cast_weaker_operand(l->type(), r->type(), other);



	return;
}
[[maybe_unused]] datatype* operand::lvalue_to_rvalue()
{
	if (type == Type::RVALUE)
		return std::get<rvalue>(value).get();


	//value.emplace<variable*>(std::get<variable*>(value));

	auto lval = std::get<variable*>(value);

	if (lval->initialized == false)
		throw runtime_error(_operand, "use of an uninitialized variable '%s'", lval->identifier.c_str());

	//value.emplace(1);

	auto dtype = lval->value.get();

	switch (lval->value->type()) {
	case datatype_e::bool_t:
		value = datatype::cast<bool_dt>(dtype);
		break;
	case datatype_e::int_t:
		value = datatype::cast<integer_dt>(dtype);
		break;
	case datatype_e::double_t:
		value = datatype::cast<double_dt>(dtype);
		break;
	}

	type = Type::RVALUE;
	//std::get<variable*>(value) = nullptr;

	return std::get<rvalue>(value).get();
}
datatype* operand::get_value()
{
	return type == Type::RVALUE ? std::get<rvalue>(value).get() : std::get<variable*>(value)->value.get();
}