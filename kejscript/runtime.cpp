#include "pch.hpp"

#include "runtime.hpp"

runtime::runtime(const VectorTokenPtr& _tokens, const std::unordered_map<std::string, function_def>& table) : tokens(_tokens), function_table(table)
{
	
}

void runtime::execute()
{
	auto it = tokens.begin();
	auto end = tokens.end();

	while (it != end) {



		++it;
	}

}