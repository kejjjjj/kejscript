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
	string_t,
	char_t
};
struct bool_dt;
struct integer_dt;
struct double_dt;
struct string_dt;

struct datatype
{
	virtual ~datatype() = default;

	virtual size_t size_of() const noexcept(true) = 0;
	virtual datatype_e type() const noexcept(true) = 0;

	virtual std::string type_str() const noexcept(true) = 0;
	virtual std::string value_str() const noexcept(true) = 0;
	virtual bool is_integral() const noexcept(true) = 0;
	virtual bool is_numeric() const noexcept(true) = 0;
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

using datatype_ptr = std::unique_ptr<datatype>;

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
	bool is_numeric() const noexcept(true) override { return true; }
	bool bool_convertible() const noexcept(true) override { return true; }

	//bool_dt operator-() const {
	//	auto result = -this->get();
	//	return (decltype(*this))(result);
	//}
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
	bool_dt operator!=(const bool_dt& other) const {
		bool result = this->get() != other.get();
		return (decltype(*this))(result);
	}
	bool_dt operator<(const bool_dt& other) const {
		bool result = this->get() < other.get();
		return (decltype(*this))(result);
	}
	bool_dt operator>(const bool_dt& other) const {
		bool result = this->get() > other.get();
		return (decltype(*this))(result);
	}
	bool_dt operator*(const bool_dt& other) const {
		bool result = this->get() * other.get();
		return (decltype(*this))(result);
	}
	//bool_dt operator/(const bool_dt& other) const {
	//	bool result = this->get() / other.get();
	//	return (decltype(*this))(result);
	//}
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
	bool is_numeric() const noexcept(true) override { return true; }
	bool bool_convertible() const noexcept(true) override { return true; }

	void set(int32_t v) { *reinterpret_cast<int32_t*>(value.data()) = v; }

	auto operator-() const {
		auto result = -this->get();
		return (decltype(*this))(result);
	}
	auto operator+(const integer_dt& other) const {
		auto result = this->get() + other.get();
		return (decltype(*this))(result);
	}
	auto operator-(const integer_dt& other) const {
		auto result = this->get() - other.get();
		return (decltype(*this))(result);
	}
	auto operator==(const integer_dt& other) const {
		auto result = this->get() == other.get();
		return (decltype(*this))(result);
	}
	auto operator!=(const integer_dt& other) const {
		auto result = this->get() != other.get();
		return (decltype(*this))(result);
	}
	auto operator<(const integer_dt& other) const {
		auto result = this->get() < other.get();
		return (decltype(*this))(result);
	}
	auto operator>(const integer_dt& other) const {
		auto result = this->get() > other.get();
		return (decltype(*this))(result);
	}
	auto operator%(const integer_dt& other) const {
		auto result = this->get() % other.get();
		return (decltype(*this))(result);
	}
	auto operator*(const integer_dt& other) const {
		auto result = this->get() * other.get();
		return (decltype(*this))(result);
	}
	auto operator/(const integer_dt& other) const {
		auto result = this->get() / other.get();
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
	bool is_numeric() const noexcept(true) override { return true; }
	
	
	bool bool_convertible() const noexcept(true) override { return true; }
	void set(double v) { *reinterpret_cast<double*>(value.data()) = v; }

	auto operator-() const {
		auto result = -this->get();
		return (decltype(*this))(result);
	}
	auto operator+(const double_dt& other) const {
		auto result = this->get() + other.get();
		return (decltype(*this))(result);
	}
	auto operator-(const double_dt& other) const {
		auto result = this->get() - other.get();
		return (decltype(*this))(result);
	}
	auto operator==(const double_dt& other) const {
		auto result = this->get() == other.get();
		return (decltype(*this))(result);
	}
	auto operator!=(const double_dt& other) const {
		auto result = this->get() != other.get();
		return (decltype(*this))(result);
	}
	auto operator<(const double_dt& other) const {
		auto result = this->get() < other.get();
		return (decltype(*this))(result);
	}
	auto operator>(const double_dt& other) const {
		auto result = this->get() > other.get();
		return (decltype(*this))(result);
	}
	auto operator*(const double_dt& other) const {
		auto result = this->get() * other.get();
		return (decltype(*this))(result);
	}
	auto operator/(const double_dt& other) const {
		auto result = this->get() / other.get();
		return (decltype(*this))(result);
	}
};
struct string_dt : public datatype
{
	~string_dt() = default;

	string_dt(const std::vector<char>& v) {
		value = v;
	};
	string_dt(const std::string& s) {
		set(s);
	}
	size_t size_of() const noexcept(true) override { return value.size(); };
	datatype_e type() const noexcept(true) override { return datatype_e::string_t; }
	const auto& get() const noexcept { return value; }
	const char* get_ptr() const noexcept { return value.data(); }

	std::string type_str() const noexcept(true) override { return "string"; }
	std::string value_str() const noexcept(true) override { return get_ptr(); }
	bool is_integral() const noexcept(true) override { return false; }
	bool is_numeric() const noexcept(true) override { return false; }
	bool bool_convertible() const noexcept(true) override { return false; }

	void set(const std::vector<char>& v) { value = v; }
	void set(const std::string& v) {
		value.clear();
		std::for_each(v.begin(), v.end(), [&](char c) { value.push_back(c); });
	}

};
struct char_dt : public datatype
{
	~char_dt() = default;

	char_dt(char v) {
		value = std::vector<char>(1, v);
	};

	size_t size_of() const noexcept(true) override { return sizeof(char); };
	datatype_e type() const noexcept(true) override { return datatype_e::char_t; }
	const char get() const noexcept { return value[0]; }

	std::string type_str() const noexcept(true) override { return "char"; }
	std::string value_str() const noexcept(true) override { std::string str; str.push_back(get()); return str; }
	bool is_integral() const noexcept(true) override { return true; }
	bool is_numeric() const noexcept(true) override { return true; }
	bool bool_convertible() const noexcept(true) override { return true; }
	void set(char v) { value = std::vector<char>(1, v); }

	auto operator-() const {
		char result = -this->get();
		return (decltype(*this))(result);
	}
	auto operator+(const char_dt& other) const {
		char result = this->get() + other.get();
		return (decltype(*this))(result);
	}
	auto operator-(const char_dt& other) const {
		char result = this->get() - other.get();
		return (decltype(*this))(result);
	}
	auto operator==(const char_dt& other) const {
		auto result = this->get() == other.get();
		return (decltype(*this))(result);
	}
	auto operator!=(const char_dt& other) const {
		auto result = this->get() != other.get();
		return (decltype(*this))(result);
	}
	auto operator<(const char_dt& other) const {
		auto result = this->get() < other.get();
		return (decltype(*this))(result);
	}
	auto operator>(const char_dt& other) const {
		auto result = this->get() > other.get();
		return (decltype(*this))(result);
	}
	auto operator%(const char_dt& other) const {
		char result = this->get() % other.get();
		return (decltype(*this))(result);
	}
	auto operator*(const char_dt& other) const {
		char result = this->get() * other.get();
		return (decltype(*this))(result);
	}
	auto operator/(const char_dt& other) const {
		char result = this->get() / other.get();
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
	case datatype_e::char_t:
		return static_cast<CastType>(datatype::cast_normal<char_dt>(&target).get());
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
	case datatype_e::char_t:
		return static_cast<CastType>(datatype::cast_normal<char_dt>(&target).get());
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