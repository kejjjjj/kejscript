#pragma once

//#include "pch.hpp"

#include "punctuation.hpp"
#include "linting_exceptions.hpp"
#include "linting_scope.hpp"

struct undefined_variable
{
	std::string identifier;
	ListTokenPtr::iterator location;
	size_t num_args = 0;
	bool function = false;
};
struct linting_data
{
	static linting_data& getInstance() { static linting_data d; return d; }
	linting_scope* active_scope = 0;
	function_def current_function;
	ListTokenPtr* tokens = 0;
	void validate(ListTokenPtr::iterator it, ListTokenPtr::iterator to);

	//void remove_token(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to) {
	//	bool is_end = (to == tokens->end());
	//	auto end_diff = std::distance(tokens->begin(), to);
	//	it = std::prev(tokens->erase(it, std::next(it)));
	//	to = is_end ? tokens->end() : std::next(tokens->begin(),  end_diff - 1);
	//}

	bool function_exists(const std::string& s) const {
		return function_table.find(s) != function_table.end();
	}
	bool function_declare(function_def&& def) {
		
		if (function_exists(def.identifier))
			return false;

		LOG("declaring the function '" << def.identifier << "' with " << def.parameters.size() << " parameters!\n");

		function_table.insert({ def.identifier, def });
		return true;
	}

	void test_all_undefined() {

		for (auto& undefined_var : undefined_variables) {

			const auto& function = function_table.find(undefined_var.identifier);

			if (function == function_table.end())
				throw linting_error(undefined_var.location->get(), "unknown function '%s'", undefined_var.identifier.c_str());

			if (undefined_var.num_args != function->second.parameters.size()) {
				throw linting_error(undefined_var.location->get(), "no instance of the function '%s' accepts %i arguments", undefined_var.identifier.c_str(), (unsigned __int64)undefined_var.num_args);

			}
		}
		undefined_variables.clear();
	}

	std::unordered_map<std::string, function_def> function_table;
	std::list<undefined_variable> undefined_variables;


};

struct linting_expression
{
	token_t* identifier = 0;
};

struct l_expression_context
{
	l_expression_context(const expression_token_stack& _stack) : stack(_stack) {}

	linting_expression expression;
	//std::vector<linting_expression> expressions;
	expression_token_stack stack;
	std::unique_ptr<undefined_variable> undefined_var;
	NO_COPY_CONSTRUCTOR(l_expression_context);
};

struct l_expression_results
{
	ListTokenPtr::iterator it;
	size_t num_evaluations = 0;
};

codeblock_parser_type get_codeblock_type(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);

void evaluate_identifier_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);

void evaluate_declaration_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
[[nodiscard]] l_expression_results evaluate_expression_sanity(ListTokenPtr::iterator it, ListTokenPtr::iterator to, const expression_token_stack& = expression_token_stack());
void evaluate_function_declaration_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_return_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
std::unique_ptr<code_block>& evaluate_if_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_else_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_while_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);

[[nodiscard]] ListTokenPtr::iterator evaluate_subscript_sanity(ListTokenPtr::iterator begin, ListTokenPtr::iterator& end, l_expression_context& context);
[[nodiscard]] ListTokenPtr::iterator  evaluate_function_call_sanity(ListTokenPtr::iterator begin, ListTokenPtr::iterator& end, l_expression_context& context);
