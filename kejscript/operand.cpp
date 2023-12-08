#include "operand.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

operand::operand(const expression_t& expr) : value()
{
	using rvalue = std::unique_ptr<datatype>;


	static_assert(std::variant_size_v<decltype(value)> > 0, "No alternatives in the variant");


	std::get<rvalue>(value) = nullptr;
	if (expr.identifier->is_identifier()) {
		const auto& table = runtime::get_instance().variables;
		std::get<variable*>(value) = table.find(expr.identifier->string)->second.get();


		type = Type::LVALUE;

		return;
	}
	else {
		type = Type::RVALUE;
		switch (expr.identifier->tt) {
		case tokentype_t::NUMBER_LITERAL:
			std::get<rvalue>(value) = std::make_unique<integer_dt>(std::stoi(expr.identifier->string));
			break;
		case tokentype_t::FLOAT_LITERAL:
			std::get<rvalue>(value) = std::make_unique<float_dt>(std::stof(expr.identifier->string));
			break;
		default:
			throw runtime_error(expr.identifier, "huh?");
		}
	}
	

	//todo: evaluate postfix and prefix



}
datatype* operand::lvalue_to_rvalue()
{
	if (type == Type::RVALUE)
		return std::get<rvalue>(value).get();

	switch (std::get<variable*>(value)->value->type()) {
	case datatype_e::int_t:
		std::get<rvalue>(value) =
			std::make_unique<integer_dt>(dynamic_cast<integer_dt*>(std::get<variable*>(value)->value.get())->get());
		break;
	case datatype_e::float_t:
		std::get<rvalue>(value) =
			std::make_unique<float_dt>(dynamic_cast<float_dt*>(std::get<variable*>(value)->value.get())->get());
		break;
	}

	type = Type::RVALUE;
	std::get<variable*>(value) = nullptr;

	return std::get<rvalue>(value).get();
}