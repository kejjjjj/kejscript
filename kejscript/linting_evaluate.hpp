#pragma once

//#include "pch.hpp"

#include "punctuation.hpp"
#include "linting_exceptions.hpp"
#include "linting_scope.hpp"

#include "function_sanity.hpp"

struct linting_data
{
	static linting_data& getInstance() { static linting_data d; return d; }
	linting_scope* active_scope = 0;
	struct_def* active_struct = 0;
	function_block* current_function = 0;
	ListTokenPtr* tokens = 0;
	std::unordered_map<std::string, function_def> existing_funcs;
	std::unordered_map<std::string, std::unique_ptr<struct_def>> structs;

	void validate(ListTokenPtr::iterator it, ListTokenPtr::iterator to);

	function_block* get_function(const std::string& name) {

		if (!function_exists(name))
			return nullptr;

		return unevaluated_functions.find(name)->second.target;
	}
	bool function_exists(const std::string& s) const { return unevaluated_functions.find(s) != unevaluated_functions.end(); }
	auto& function_declare(std::unique_ptr<function_block>& func) {
		
		if (function_exists(func->def.identifier))
			throw linting_error("the function '%s' already exists", func->def.identifier.c_str());

		LOG("declaring the function '" << func->def.identifier << "' with " << func->def.parameters.size() << " parameters!\n");
		return function_table.insert(function_table.end(), { func->def.identifier, std::move(func) })->second;
		
	}
	bool struct_exists(const std::string& s) const { return unevaluated_structs.find(s) != unevaluated_structs.end(); }
	auto get_struct_data(const std::string& s) const { return unevaluated_structs.find(s)->second.def; }
	auto& struct_declare(std::unique_ptr<struct_def>& _struct) {

		if (struct_exists(_struct->identifier))
			throw linting_error("the struct '%s' already exists", _struct->identifier.c_str());

		LOG("declaring the struct '" << _struct->identifier << "' with " << _struct->initializers.size() << " variables!\n");
		
		return structs.insert(structs.end(), { _struct->identifier, std::move(_struct) })->second;
	}
	std::unordered_map<std::string, std::unique_ptr<function_block>> function_table;
	std::unordered_map<std::string, unevaluated_function> unevaluated_functions;
	std::unordered_map<std::string, unevaluated_struct> unevaluated_structs;

};

struct linting_expression
{
	token_t* identifier = 0;
};


struct l_expression_results
{
	ListTokenPtr::iterator it;
	size_t num_evaluations = 0;

};

void evaluate_identifier_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);

void evaluate_declaration_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
[[nodiscard]] l_expression_results evaluate_expression_sanity(ListTokenPtr::iterator it, ListTokenPtr::iterator to, std::unique_ptr<expression_block>& block, const expression_token_stack& = expression_token_stack());
void evaluate_function_declaration_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);

//assumes that "it" is the token after "fn"
std::unique_ptr<function_block> parse_function_declaration(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_return_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_if_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to, const std::unique_ptr<conditional_block>&);
void evaluate_else_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_while_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_for_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to);
void evaluate_struct_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end);

[[nodiscard]] ListTokenPtr::iterator evaluate_subscript_sanity(
	ListTokenPtr::iterator begin, 
	ListTokenPtr::iterator& end, 
	expression_context& context,
	operatorlist::iterator& it);
[[nodiscard]] ListTokenPtr::iterator  evaluate_function_call_sanity(
	ListTokenPtr::iterator begin,
	ListTokenPtr::iterator& end,
	expression_context& context,
	operatorlist::iterator& it
	);
[[nodiscard]] ListTokenPtr::iterator  evaluate_member_access_sanity(
	ListTokenPtr::iterator begin,
	ListTokenPtr::iterator& end,
	operatorlist::iterator& it
);


template <typename t>
inline t* move_block_to_current_context(std::unique_ptr<t>& block)
{
	auto& data = linting_data::getInstance();
	linting_scope* scope = data.active_scope;

	code_block* cblock = nullptr;
	

	//function scope root
	if (scope->lower_scope->is_global_scope() || scope->lower_scope->is_struct_scope()) {
		data.current_function->add_instruction(block);
		cblock = data.current_function->instructions.back().get();
		cblock->owner = data.current_function;
		return dynamic_cast<t*>(cblock);
	}

	//currently inside of a code block
	
	bool increase_nesting = block->type() >= code_block_e::CONDITIONAL && block->type() <= code_block_e::WHILE;

	cblock = data.current_function->blocks.back();

	//a conditional block can have chained blocks so iterate to the last block
	if (cblock->type() == code_block_e::CONDITIONAL) {
		conditional_block* nextblock = dynamic_cast<conditional_block*>(cblock);
		while (nextblock->next) {
			nextblock = nextblock->next.get();
		}

		cblock = nextblock;
	}

	for(size_t i = 0; i < data.current_function->nest_depth; i++)
		cblock = cblock->contents.back().get();

	cblock->contents.push_back(std::move(block));

	//a statement that will increase the nesting!
	if (increase_nesting) {
		++data.current_function->nest_depth;
	}

	if(cblock->contents.size())
		cblock = cblock->contents.back().get();

	cblock->owner = data.current_function;
	return dynamic_cast<t*>(cblock);
}