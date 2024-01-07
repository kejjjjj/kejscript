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
	bool_t,
	int_t,
	double_t,
};
struct bool_dt;
struct integer_dt;
struct double_dt;

struct datatype
{
	virtual ~datatype() = default;

	virtual size_t size_of() const noexcept(true) = 0;
	virtual datatype_e type() const noexcept(true) = 0;

	virtual std::string type_str() const noexcept(true) = 0;
	virtual std::string value_str() const noexcept(true) = 0;
	virtual bool is_integral() const noexcept(true) = 0;
	virtual bool bool_convertible() const noexcept(true) = 0;

	void implicit_cast(datatype* other);

	template<typename T>
	static auto cast(datatype*d) {
		return std::make_unique<T>(dynamic_cast<T*>(d)->get());
	}
	template<typename T>
	static constexpr auto& cast_normal(const datatype* d) {
		return *dynamic_cast<const T*>(d);
	}
	template<typename T> 
	constexpr T& getvalue() noexcept;

	template<typename Base, typename CastType>
	static constexpr Base create_type(const datatype& target);
	template<typename Base, typename CastType>
	static constexpr Base create_type_copy(const Base& target);
	template<typename Base, typename CastType>
	static constexpr std::unique_ptr<Base> create_type_ptr(datatype& target);

	std::vector<char> value;
protected:
	/*std::vector<char> value;*/

};
using rvalue = std::unique_ptr<datatype>;
struct bool_dt : public datatype
{
	~bool_dt() = default;

	bool_dt(bool v) {
		value.resize(size_of());
		*reinterpret_cast<bool*>(value.data()) = v;
	};

	size_t size_of() const noexcept(true) override { return sizeof(bool); };
	datatype_e type() const noexcept(true) override { return datatype_e::bool_t; }
	const bool get() const noexcept { return *reinterpret_cast<const bool*>(value.data()); }
	void set(bool v) { *reinterpret_cast<bool*>(value.data()) = v; }
	std::string type_str() const noexcept(true) override { return "bool"; }
	std::string value_str() const noexcept(true) override { return get() == true ? "true" : "false"; }
	bool is_integral() const noexcept(true) override { return true; }
	bool bool_convertible() const noexcept(true) override { return true; }

	bool_dt operator+(const bool_dt& other) const {
		bool result = this->get() + other.get();
		return (decltype(*this))(result);
	}
	bool_dt operator-(const bool_dt& other) const {
		bool result = this->get() - other.get();
		return (decltype(*this))(result);
	}
	bool_dt operator==(const bool_dt& other) const {
		bool result = this->get() == other.get();
		return (decltype(*this))(result);
	}
	bool_dt operator<(const bool_dt& other) const {
		bool result = this->get() < other.get();
		return (decltype(*this))(result);
	}
	bool_dt operator*(const bool_dt& other) const {
		bool result = this->get() * other.get();
		return (decltype(*this))(result);
	}
};

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
	std::string type_str() const noexcept(true) override { return "int"; }
	std::string value_str() const noexcept(true) override { return std::to_string(get()); }
	bool is_integral() const noexcept(true) override { return true; }
	bool bool_convertible() const noexcept(true) override { return true; }
	void set(int32_t v) { *reinterpret_cast<int32_t*>(value.data()) = v; }


	integer_dt operator+(const integer_dt& other) const {
		auto result = this->get() + other.get();
		LOG(std::format("{} + {} = {}\n", this->get(), other.get(), this->get() + other.get()));
		return (decltype(*this))(result);
	}
	integer_dt operator-(const integer_dt& other) const {
		auto result = this->get() - other.get();
		return (decltype(*this))(result);
	}
	integer_dt operator==(const integer_dt& other) const {
		bool result = this->get() == other.get();
		return (decltype(*this))(result);
	}
	integer_dt operator<(const integer_dt& other) const {
		bool result = this->get() < other.get();
		LOG(std::format("{} < {} = {}\n", this->get(), other.get(), this->get() < other.get()));
		return (decltype(*this))(result);
	}
	integer_dt operator%(const integer_dt& other) const {
		int result = this->get() % other.get();
		LOG(std::format("{} % {} = {}\n", this->get(), other.get(), this->get() % other.get()));
		return (decltype(*this))(result);
	}
	integer_dt operator*(const integer_dt& other) const {
		int result = this->get() * other.get();
		return (decltype(*this))(result);
	}
};

struct double_dt : public datatype
{
	~double_dt() = default;

	double_dt(double v) {
		value.resize(size_of());
		*reinterpret_cast<double*>(value.data()) = v;
	};

	size_t size_of() const noexcept(true) override { return sizeof(double); };
	datatype_e type() const noexcept(true) override { return datatype_e::double_t; }
	const double get() const noexcept { return *reinterpret_cast<const double*>(value.data()); }
	std::string type_str() const noexcept(true) override { return "double"; }
	std::string value_str() const noexcept(true) override { return std::to_string(get()); }
	bool is_integral() const noexcept(true) override { return false; }
	bool bool_convertible() const noexcept(true) override { return true; }
	void set(double v) { *reinterpret_cast<double*>(value.data()) = v; }

	double_dt operator+(const double_dt& other) const {
		auto result = this->get() + other.get();
		return (decltype(*this))(result);
	}
	double_dt operator-(const double_dt& other) const {
		auto result = this->get() - other.get();
		return (decltype(*this))(result);
	}
	double_dt operator==(const double_dt& other) const {
		bool result = this->get() == other.get();
		return (decltype(*this))(result);
	}
	double_dt operator<(const double_dt& other) const {
		bool result = this->get() < other.get();
		return (decltype(*this))(result);
	}
	double_dt operator*(const double_dt& other) const {
		auto result = this->get() * other.get();
		return (decltype(*this))(result);
	}
};

template<typename Base, typename CastType>
inline constexpr Base datatype::create_type(const datatype& target)
{
	switch (target.type()) {
	case datatype_e::bool_t:
		return static_cast<CastType>(datatype::cast_normal<bool_dt>(&target).get());
	case datatype_e::int_t:
		return static_cast<CastType>(datatype::cast_normal<integer_dt>(&target).get());
	case datatype_e::double_t:
		return static_cast<CastType>(datatype::cast_normal<double_dt>(&target).get());
	}

	throw std::logic_error("yea");
}
template<typename Base, typename CastType>
inline constexpr Base datatype::create_type_copy(const Base& target)
{
	switch (target.type()) {
	case datatype_e::bool_t:
		return static_cast<CastType>(datatype::cast_normal<bool_dt>(&target).get());
	case datatype_e::int_t:
		return static_cast<CastType>(datatype::cast_normal<integer_dt>(&target).get());
	case datatype_e::double_t:
		return static_cast<CastType>(datatype::cast_normal<double_dt>(&target).get());
	}

	throw std::logic_error("yea");
}
template<typename Base, typename CastType>
inline constexpr std::unique_ptr<Base> datatype::create_type_ptr(datatype& target)
{
	return std::make_unique<Base>(create_type<Base, CastType>(target));
}

template<typename T>
inline constexpr T& datatype::getvalue() noexcept {
	return *dynamic_cast<T*>(this);
}