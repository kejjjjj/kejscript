#pragma once

#include "pch.hpp"

#include "runtime_expression.hpp"
#include "operand.hpp"


struct expression_node {

	enum class Type {
		OPERAND,
		OPERATOR
	};
	struct operator_s
	{
		punctuation_e punc = punctuation_e::P_UNKNOWN;
		OperatorPriority priority = OperatorPriority::FAILURE;
	};
	expression_node() = delete;
	expression_node(const expression_t& expr) : _operand(std::unique_ptr<operand>(new operand(expr))) {
		type = Type::OPERAND;
	}
	expression_node(const operator_s op) {
		_operator.punc = op.punc;
		_operator.priority = op.priority;
		type = Type::OPERATOR;
	}

	union {
		std::unique_ptr<operand> _operand;
		operator_s _operator{ punctuation_e::P_UNKNOWN, OperatorPriority::FAILURE };
	};

	~expression_node() {
		if (type == Type::OPERAND) {
			_operand.reset();
		}
	}

	expression_node& operator=(const expression_node&) = delete;
	expression_node(const expression_node&) = delete;

	Type type = Type::OPERAND;
};

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
	static void arithmetic_addition(expression_node&, expression_node&);

	std::unordered_map<punctuation_e, std::function<void(expression_node&, expression_node&)>> eval_functions;

};