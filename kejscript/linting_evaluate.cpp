#include "pch.hpp"

#include "linting_evaluate.hpp"

void linting_data::validate(ListTokenPtr::iterator it, ListTokenPtr::iterator end)
{
	active_scope = new linting_scope;

	auto& codepos = it;

	while (codepos != end) {
		evaluate_identifier_sanity(codepos, end);
		++codepos;
	}

	for (auto& structure : unevaluated_structs) {
		structure.second.evaluate(end);
	}

	for (auto& func : unevaluated_functions) {
		func.second.evaluate(end, true);
	}

	if (!active_scope->is_global_scope())
		throw linting_error((--end)->get(), "expected to find a '}'");

	//find main entry 
	auto entry = function_table.find("main");

	if (entry == function_table.end())
		throw linting_error("didn't find the main() function");

	if(entry->second->def.parameters.size() != NULL)
		throw linting_error("the main() function should not have parameters");



}

void evaluate_identifier_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to)
{
	auto& data = linting_data::getInstance();

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN)) {
		data.active_scope = linting_create_scope_without_range(data.active_scope);
		return;
	}else if (it->get()->is_operator(P_CURLYBRACKET_CLOSE)) {
		data.active_scope = linting_delete_scope(it, data.active_scope);
		return;
	}

	switch (it->get()->tt) {
	case tokentype_t::DEF:
		return evaluate_declaration_sanity(it, to);
	case tokentype_t::FN:
		return evaluate_function_declaration_sanity(it, to);
	case tokentype_t::RETURN:
		return evaluate_return_sanity(it, to);
	case tokentype_t::IF:
		return evaluate_if_sanity(it, to, nullptr);
	case tokentype_t::ELSE:
		return evaluate_else_sanity(it, to);
	case tokentype_t::WHILE:
		return evaluate_while_sanity(it, to);
	case tokentype_t::FOR:
		return evaluate_for_sanity(it, to);
	case tokentype_t::STRUCT:
		return evaluate_struct_sanity(it, to);
	}

	auto block = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, to, block).it;
	move_block_to_current_context(block);

}