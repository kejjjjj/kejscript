#pragma once

#include "pch.hpp"
#include "datatype.hpp"

struct variable
{
	variable(const std::string i) : identifier(i) {};
	~variable() = default;

	std::string identifier;
	std::unique_ptr<datatype> value;
	bool initialized = false;

private:
	variable& operator=(const variable&) = delete;
	variable(const variable&) = delete;
};
