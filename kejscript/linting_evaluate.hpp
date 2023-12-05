#pragma once

//#include "pch.hpp"

#include "punctuation.hpp"
#include "linting_exceptions.hpp"
#include "linting_scope.hpp"

struct undefined_variable
{
	std::string identifier;
	VectorTokenPtr::iterator location;
	size_t num_args = 0;
	bool function = false;
};
struct linting_data
{
	static linting_data& getInstance() { static linting_data d; return d; }
	linting_scope* active_scope = 0;

	void validate(VectorTokenPtr::iterator it, VectorTokenPtr::iterator to);

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
	std::vector<token_t*> prefix;
	token_t* identifier = 0;
	std::vector<token_t*> postfix;
	bool op = false;

	bool empty() const noexcept {
		return prefix.empty() && identifier == nullptr;
	}
};

struct expression_context
{
	expression_context(const expression_token_stack& _stack) : stack(_stack) {}

	linting_expression expression;
	std::vector<linting_expression> expressions;
	expression_token_stack stack;
	std::unique_ptr<undefined_variable> undefined_var;
	expression_context(const expression_context&) = delete;
	expression_context& operator=(const expression_context&) = delete;

};

struct expression_results
{
	VectorTokenPtr::iterator it;
	size_t num_evaluations = 0;
};

codeblock_parser_type get_codeblock_type(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);

void evaluate_identifier_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);

void evaluate_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
[[nodiscard]] expression_results evaluate_expression_sanity(VectorTokenPtr::iterator it, VectorTokenPtr::iterator to, const expression_token_stack& = expression_token_stack());
void evaluate_function_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
void evaluate_return_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
void evaluate_if_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
void evaluate_else_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);

[[nodiscard]] VectorTokenPtr::iterator evaluate_subscript_sanity(VectorTokenPtr::iterator begin, VectorTokenPtr::iterator& end, expression_context& context);
[[nodiscard]] VectorTokenPtr::iterator  evaluate_function_call_sanity(VectorTokenPtr::iterator begin, VectorTokenPtr::iterator& end, expression_context& context);
