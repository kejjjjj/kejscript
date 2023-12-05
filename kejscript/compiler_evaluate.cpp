#include "pch.hpp"
codeblock_parser_type get_codeblock_type(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	if (it == end)
		throw compile_error("get_codeblock_type(): it == end");

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

void evaluate_identifier_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to)
{

	LOG("calling evaluate_identifier_sanity()\n");

	if (it->get()->tt == tokentype_t::DEF) {
		return evaluate_declaration_sanity(it, to);
	}else if (it->get()->tt == tokentype_t::FN) {
		return evaluate_function_declaration_sanity(it, to);
	}
	else if (it->get()->tt == tokentype_t::RETURN) {
		return evaluate_return_sanity(it, to);
	}

	it = evaluate_expression_sanity(it, to);



}