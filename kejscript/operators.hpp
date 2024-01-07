#pragma once

#include "pch.hpp"
#include "operand.hpp"
#include "linting_exceptions.hpp"

template<typename T>
constexpr std::unique_ptr<operand> create_rvalue(const T& t)
{
	std::unique_ptr<datatype> v = std::make_unique<T>(t);
	return std::make_unique<operand>(std::move(v));
}
inline std::unique_ptr<operand> create_lvalue(variable* v)
{
	return std::make_unique<operand>(v);
}
class evaluation_functions
{
public:
	static void initialize_functions();

	using funcptr = std::unique_ptr<operand>(*)(operand&, operand&);
	using unaryfuncptr = void(*)(operand&);

	static funcptr find_function(const punctuation_e p) {
		const auto found = eval_functions.find(p);

		if (found == eval_functions.end()) {
			throw linting_error("unsupported operator");
		}
		return found->second;
	}
	static unaryfuncptr find_unary_function(const punctuation_e p) {
		const auto found = unary_functions.find(p);

		if (found == unary_functions.end()) {
			throw linting_error("unsupported operator");
		}
		return found->second;
	}
	static void assign_to_lvalue(variable*, operand&);

private:

	static std::unique_ptr<operand> arithmetic_addition(operand&, operand&);
	static std::unique_ptr<operand> arithmetic_subtraction(operand&, operand&);

	static std::unique_ptr<operand> assignment(operand&, operand&);
	static std::unique_ptr<operand> less_than(operand&, operand&);
	static std::unique_ptr<operand> modulo(operand&, operand&);
	static std::unique_ptr<operand> multiplication(operand&, operand&);
	static std::unique_ptr<operand> equality(operand&, operand&);

	static std::unordered_map<punctuation_e, funcptr> eval_functions;

	static void increment(operand&);

	static std::unordered_map<punctuation_e, unaryfuncptr> unary_functions;


};