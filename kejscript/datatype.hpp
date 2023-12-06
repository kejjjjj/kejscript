#pragma once

#include "pch.hpp"

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

struct integer_t : public datatype
{
	~integer_t() = default;

	integer_t(int32_t v) {
		value.resize(size_of());
		*reinterpret_cast<int32_t*>(value.data()) = v;
	};

	size_t size_of() const noexcept(true) override { return sizeof(int32_t); };
	datatype_e type() const noexcept(true) override { return datatype_e::int_t; }
	const int32_t get() const noexcept { return *reinterpret_cast<const int32_t*>(value.data()); }

	integer_t operator+(const integer_t& other) {
		return integer_t(this->get() + other.get());
	}

};