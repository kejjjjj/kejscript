#include "pch.hpp"

#include "linting_evaluate.hpp"

void linting_data::validate(ListTokenPtr::iterator it, ListTokenPtr::iterator end)
{
	active_scope = new linting_scope;

	auto& codepos = it;

	while (codepos != end) {
		const auto parser_required = get_codeblock_type(codepos, end);

		switch (parser_required) {
		case codeblock_parser_type::CREATE_SCOPE:
			active_scope = linting_create_scope_without_range(active_scope);
			break;
		case codeblock_parser_type::DELETE_SCOPE:
			active_scope = linting_delete_scope(it, active_scope);
			break;
		default:
			evaluate_identifier_sanity(codepos, end);
			break;
		}

		if (codepos == end)
			throw linting_error((--end++)->get(), "this is a temporary error but it happened because an unexpected eof was encountered!");

		++codepos;
	}

	if (!active_scope->is_global_scope())
		throw linting_error((--end)->get(), "expected to find a '}'");

	test_all_undefined();

	//find main entry 
	auto entry = function_table.find("main");

	if (entry == function_table.end())
		throw linting_error("didn't find the main() function");

	if(entry->second->def.parameters.size() != NULL)
		throw linting_error("the main() function should not have parameters");



}

codeblock_parser_type get_codeblock_type(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	if (it == end)
		throw linting_error("get_codeblock_type(): it == end");

	if (it->get()->is_punctuation()) {
		auto punc = dynamic_cast<punctuation_token_t*>(it->get())->punc;

		if (punc == punctuation_e::P_CURLYBRACKET_OPEN) {
			return codeblock_parser_type::CREATE_SCOPE;
		}else if (punc == punctuation_e::P_CURLYBRACKET_CLOSE) {
			return codeblock_parser_type::DELETE_SCOPE;
		}
	}

	return codeblock_parser_type::DEFAULT; //hardcoded to this for now

}

void evaluate_identifier_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& to)
{

	//LOG("calling evaluate_identifier_sanity()\n");

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
	}

	auto block = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, to, block).it;
	move_block_to_current_context(block);

}