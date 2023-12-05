#pragma once

#include "pch.hpp"

struct runtime
{
	runtime(const VectorTokenPtr& _tokens, const std::unordered_map<std::string, function_def>& table);
	
	void execute();

private:
	function_def* entry_point = 0;
	std::unordered_map<std::string, function_def> function_table;
	VectorTokenPtr tokens;
};