#pragma once

#include "pch.hpp"

#include "variable.hpp"

struct runtime
{
	runtime() = default;
	static runtime& get_instance() { static runtime r; return r; }
	void initialize(const VectorTokenPtr::iterator begin, const VectorTokenPtr::iterator end, const std::unordered_map<std::string, function_def>& table);

	void execute();

	void declare_variable(const std::string& i) {
		variables.insert({ i, std::move(std::make_unique<variable>()) });
	}

	std::unordered_map<std::string, std::unique_ptr<variable>> variables;

private:
	function_def* entry_point = 0;
	std::unordered_map<std::string, function_def> function_table;
	VectorTokenPtr::iterator _begin;
	VectorTokenPtr::iterator _end;


	runtime& operator=(const runtime&) = delete;
	runtime(const runtime&) = delete;
};