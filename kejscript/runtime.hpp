#pragma once

#include "pch.hpp"

#include "variable.hpp"

struct stack_t
{
	stack_t(const VectorTokenPtr::iterator& stack_frame) : return_location(stack_frame){}

	void declare_variable(const std::string& i) {
		variables.insert({ i, std::move(std::make_unique<variable>(i)) });
		LOG("declared '" << i << "'\n");
	}
	void print_stack()
	{
		LOG("\n---- stack ----\n");
		for (auto& v : variables) {
			LOG(std::format("{}<{}> = {}\n", v.second->identifier, v.second->value->type_str(), v.second->value->value_str()));

		}

	}
	std::unordered_map<std::string, std::unique_ptr<variable>> variables;

	VectorTokenPtr::iterator return_location;

	std::unique_ptr<stack_t> function_call;
	stack_t& operator=(const stack_t&) = delete;
	stack_t(const stack_t&) = delete;
};

struct runtime
{
	runtime() = default;
	static runtime& get_instance() { static runtime r; return r; }
	void initialize(const VectorTokenPtr::iterator begin, const VectorTokenPtr::iterator end, const std::unordered_map<std::string, function_def>& table);

	void execute();

	std::unique_ptr<stack_t> stack;
private:
	
	function_def* entry_point = 0;
	std::unordered_map<std::string, function_def> function_table;
	VectorTokenPtr::iterator _begin;
	VectorTokenPtr::iterator _end;

	

	runtime& operator=(const runtime&) = delete;
	runtime(const runtime&) = delete;
};

codeblock_parser_type get_codeblock_type(VectorTokenPtr::iterator& it);
void evaluate_token(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end);