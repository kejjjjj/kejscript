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
//inline std::unique_ptr<operand> create_lvalue(std::shared_ptr<variable>& v)
//{
//	return std::make_unique<operand>(v);
//}
class evaluation_functions
{
public:
	static void initialize_functions();

	using funcptr = operand*(*)(operand*, operand*);
	using unaryfuncptr = operand*(*)(operand*);

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
			throw linting_error("unsupported unary operator");
		}
		return found->second;
	}
	static void assign_to_lvalue(std::shared_ptr<variable>& var, const operand*);
	static void assign_to_lvalue(std::shared_ptr<variable>& var, operand*);

private:

	static operand* arithmetic_addition(operand*, operand*);
	//static std::unique_ptr<operand> arithmetic_subtraction(operand&, operand&);

	static operand* assignment(operand*, operand*);
	static operand* less_than(operand*, operand*);
	/*static std::unique_ptr<operand> greater_than(operand&, operand&);
	static std::unique_ptr<operand> modulo(operand&, operand&);
	static std::unique_ptr<operand> multiplication(operand&, operand&);
	static std::unique_ptr<operand> division(operand&, operand&);

	static std::unique_ptr<operand> equality(operand&, operand&);
	static std::unique_ptr<operand> unequality(operand&, operand&);

	static std::unique_ptr<operand> logical_and(operand&, operand&);*/


	static std::unordered_map<punctuation_e, funcptr> eval_functions;

	static operand* increment(operand*);
	//static operand_ptr negate(operand_ptr&);

	static std::unordered_map<punctuation_e, unaryfuncptr> unary_functions;


};
operand* subscript(operand*, operand* expression);
