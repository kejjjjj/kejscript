#pragma once

#include "pch.hpp"

#include "variable.hpp"

struct stack_t
{
	stack_t(const ListTokenPtr::iterator& stack_frame) : return_location(stack_frame){}

	void declare_variable(const std::string& i) {
		variables.insert({ i, std::move(std::make_unique<variable>(i)) });
		//LOG("declared '" << i << "'\n");
	}

	void print_stack()
	{
		std::cout << ("\n---- stack ----\n");
		for (auto& v : variables) {
			std::cout << (std::format("{}<{}> = {}\n", v.second->identifier, v.second->value->type_str(), v.second->value->value_str()));

		}
		std::cout << ("\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n");

	}

	std::unordered_map<std::string, std::unique_ptr<variable>> variables;

	bool is_conditional_block() const noexcept { return conditional_block.get(); }

	struct block_t {
		block_t(const tokentype_t t) : blockType(t){}
		ListTokenPtr::iterator start;
		ListTokenPtr::iterator end;
		ListTokenPtr::iterator jmp;
		bool exit = false;
		tokentype_t blockType = tokentype_t::UNKNOWN;
	};

	std::unique_ptr<block_t> conditional_block = 0;
	ListTokenPtr::iterator return_location;

	std::unique_ptr<stack_t> function_call;
	stack_t& operator=(const stack_t&) = delete;
	stack_t(const stack_t&) = delete;
};

struct runtime
{
	runtime() = default;
	static runtime& get_instance() { static runtime r; return r; }
	void initialize(const ListTokenPtr::iterator begin, const ListTokenPtr::iterator end, const std::unordered_map<std::string, function_def>& table);

	void execute();

	std::unique_ptr<stack_t> stack;
private:
	
	function_def* entry_point = 0;
	std::unordered_map<std::string, function_def> function_table;
	ListTokenPtr::iterator _begin;
	ListTokenPtr::iterator _end;

	

	runtime& operator=(const runtime&) = delete;
	runtime(const runtime&) = delete;
};

codeblock_parser_type get_codeblock_type(ListTokenPtr::iterator& it);
void evaluate_token(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end);