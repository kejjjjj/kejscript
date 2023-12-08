#pragma once

#include "pch.hpp"

struct expression_t
{
	std::vector<token_t*> prefix;
	token_t* identifier = 0;
	std::vector<token_t*> postfix;

	struct operator_t {
		bool is_operator = false;
		punctuation_e punc = punctuation_e::P_UNKNOWN;
		OperatorPriority priority = OperatorPriority::FAILURE;
	}op;

	bool empty() const noexcept {
		return prefix.empty() && identifier == nullptr;
	}
};


enum class datatype_e
{
	int_t,
	float_t,
};

struct datatype
{
	virtual ~datatype() = default;

	virtual size_t size_of() const noexcept(true) = 0;
	virtual datatype_e type() const noexcept(true) = 0;

	std::vector<char> value;

};
using rvalue = std::unique_ptr<datatype>;
struct integer_dt : public datatype
{
	~integer_dt() = default;

	integer_dt(int32_t v) {
		value.resize(size_of());
		*reinterpret_cast<int32_t*>(value.data()) = v;
	};

	size_t size_of() const noexcept(true) override { return sizeof(int32_t); };
	datatype_e type() const noexcept(true) override { return datatype_e::int_t; }
	const int32_t get() const noexcept { return *reinterpret_cast<const int32_t*>(value.data()); }

	integer_dt operator+(const integer_dt& other) {
		return integer_dt(this->get() + other.get());
	}

};

struct float_dt : public datatype
{
	~float_dt() = default;

	float_dt(float v) {
		value.resize(size_of());
		*reinterpret_cast<float*>(value.data()) = v;
	};

	size_t size_of() const noexcept(true) override { return sizeof(int32_t); };
	datatype_e type() const noexcept(true) override { return datatype_e::float_t; }
	const float get() const noexcept { return *reinterpret_cast<const float*>(value.data()); }

	float_dt operator+(const float_dt& other) {
		return float_dt(this->get() + other.get());
	}

};