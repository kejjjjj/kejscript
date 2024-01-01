#pragma once

#include "pch.hpp"
#include "operand.hpp"
#include "linting_exceptions.hpp"

//struct expression_node {
//
//	enum class Type {
//		OPERAND,
//		OPERATOR
//	};
//	struct operator_s
//	{
//		operator_s() = default;
//		operator_s(punctuation_e p, OperatorPriority op) : punc(p), priority(op){}
//		void set(punctuation_e p, OperatorPriority op) noexcept(true) {
//			punc = p;
//			priority = op;
//		}
//
//		punctuation_e punc = punctuation_e::P_UNKNOWN;
//		OperatorPriority priority = OperatorPriority::FAILURE;
//	};
//	expression_node() = delete;
//	expression_node(token_t& expr) : _op(std::make_unique<operand>(expr)) {
//		type = Type::OPERAND;
//	}
//	expression_node(std::unique_ptr<operand>& expr) : _op(std::move(expr)) {
//		type = Type::OPERAND;
//	}
//	expression_node(const operator_s op) : _op(operator_s(op.punc, op.priority)) {
//		type = Type::OPERATOR;
//	}
//
//
//	std::variant<std::unique_ptr<operand>, operator_s> _op;
//
//	~expression_node() {
//		if (type == Type::OPERAND) {
//			std::get<std::unique_ptr<operand>>(_op).reset();
//		}
//	}
//
//	expression_node& operator=(const expression_node&) = delete;
//	expression_node(const expression_node&) = delete;
//
//	Type type = Type::OPERAND;
//};

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
	static funcptr find_function(const punctuation_e p) {
		const auto found = eval_functions.find(p);

		if (found == eval_functions.end()) {
			throw linting_error("unsupported operator");
		}
		return found->second;
	}
private:
	static std::unique_ptr<operand> arithmetic_addition(operand&, operand&);
	//static std::unique_ptr<expression_node> arithmetic_subtraction(expression_node&, expression_node&);

	static std::unique_ptr<operand> assignment(operand&, operand&);
	static std::unique_ptr<operand> less_than(operand&, operand&);
	static std::unique_ptr<operand> equality(operand&, operand&);


	static std::unordered_map<punctuation_e, funcptr> eval_functions;

};