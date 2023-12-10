#pragma once

#include "pch.hpp"
#include "operand.hpp"


struct expression_node {

	enum class Type {
		OPERAND,
		OPERATOR
	};
	struct operator_s
	{
		operator_s() = default;
		operator_s(punctuation_e p, OperatorPriority op) : punc(p), priority(op){}
		void set(punctuation_e p, OperatorPriority op) noexcept(true) {
			punc = p;
			priority = op;
		}

		punctuation_e punc = punctuation_e::P_UNKNOWN;
		OperatorPriority priority = OperatorPriority::FAILURE;
	};
	expression_node() = delete;
	expression_node(const expression_t& expr) : _op(std::make_unique<operand>(expr)) {
		type = Type::OPERAND;
	}
	expression_node(std::unique_ptr<operand>& expr) : _op(std::move(expr)) {
		type = Type::OPERAND;
	}
	expression_node(const operator_s op) : _op(operator_s(op.punc, op.priority)) {
		type = Type::OPERATOR;
	}


	std::variant<std::unique_ptr<operand>, operator_s> _op;

	~expression_node() {
		if (type == Type::OPERAND) {
			std::get<std::unique_ptr<operand>>(_op).reset();
		}
	}

	expression_node& operator=(const expression_node&) = delete;
	expression_node(const expression_node&) = delete;

	Type type = Type::OPERAND;
};

template<typename T>
std::unique_ptr<expression_node> create_rvalue(const T& t)
{
	std::unique_ptr<datatype> v = std::make_unique<T>(t);
	auto oper = std::make_unique<operand>(std::move(v));
	return std::make_unique<expression_node>(oper);
}
inline std::unique_ptr<expression_node> create_lvalue(variable* v)
{
	auto oper = std::make_unique<operand>(v);
	return std::make_unique<expression_node>(oper);
}
class evaluation_functions
{
public:
	static auto& getInstance() {
		static evaluation_functions instance;
		return instance;
	}

	void initialize_functions();


	auto find_function(const punctuation_e p) const {
		const auto found = eval_functions.find(p);
		return found != eval_functions.end() ? std::make_optional(found->second) : std::nullopt;
	}

private:
	static std::unique_ptr<expression_node> arithmetic_addition(expression_node&, expression_node&);
	static std::unique_ptr<expression_node> arithmetic_subtraction(expression_node&, expression_node&);

	static std::unique_ptr<expression_node> assignment(expression_node&, expression_node&);
	static std::unique_ptr<expression_node> less_than(expression_node&, expression_node&);
	static std::unique_ptr<expression_node> equality(expression_node&, expression_node&);


	std::unordered_map<punctuation_e, std::function<std::unique_ptr<expression_node>(expression_node&, expression_node&)>> eval_functions;

};