#pragma once

#include "pch.hpp"

struct token_stack
{
	token_stack() = default;
	token_stack(size_t o, size_t c) : num_open(o), num_close(c) {}
	size_t num_open = 0;
	size_t num_close = 0;
};
//this structure will stop parsing the expression based on stacked tokens
struct expression_token_stack
{
	expression_token_stack() = default;
	expression_token_stack(punctuation_e open_punc, punctuation_e close_punc) : opening(open_punc), closing(close_punc), stack(1,0){}
	void assign_to_stack_if_possible(VectorTokenPtr::iterator& it) noexcept {

		if (time_to_exit())
			return;

		auto token = it->get();

		if (!token->is_punctuation())
			return;

		const auto p = dynamic_cast<const punctuation_token_t*>(token);

		if (p->punc == opening) {
			stack.num_open++;
		}
		else if (p->punc == closing) {
			stack.num_close++;
			location = it;
		}
	}

	bool time_to_exit() const noexcept {
		return stack.num_open && stack.num_close == stack.num_open;
	}
	token_stack stack;
	punctuation_e opening = punctuation_e::P_UNKNOWN;
	punctuation_e closing = punctuation_e::P_UNKNOWN;
	VectorTokenPtr::iterator location;
};

struct compiler_function_def
{
	VectorTokenPtr::iterator location;
	std::vector<std::string> parameters;
	std::string identifier;
};

struct compiler_data
{
	static compiler_data& getInstance() { static compiler_data d; return d; }
	compiler_scope* active_scope = 0;

	bool function_exists(const std::string& s) const {
		return function_table.find(s) != function_table.end();
	}
	bool function_declare(compiler_function_def&& def) {
		
		if (function_exists(def.identifier))
			return false;

		LOG("declaring the function '" << def.identifier << "' with " << def.parameters.size() << " parameters!\n");

		function_table.insert({ def.identifier, def });
		return true;
	}
	std::unordered_map<std::string, compiler_function_def> function_table;

};

codeblock_parser_type get_codeblock_type(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);

void evaluate_identifier_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);

void evaluate_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
[[nodiscard]] VectorTokenPtr::iterator evaluate_expression_sanity(VectorTokenPtr::iterator it, VectorTokenPtr::iterator to, const expression_token_stack& = expression_token_stack());
void evaluate_function_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
void evaluate_return_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
