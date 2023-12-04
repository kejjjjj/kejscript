#include "pch.hpp"

VectorTokenPtr::iterator splice_substr_at_semicolon(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end)
{
	VectorTokenPtr::iterator new_end = it;

	while (new_end != end) {

		if (new_end->get()->is_punctuation()
			&& dynamic_cast<punctuation_token_t*>(new_end->get())->punc == punctuation_e::P_SEMICOLON)
			return new_end;

		++new_end;
	}

	throw compile_error("expected to find ';' but found end of file instead\n");
}

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
	auto end = splice_substr_at_semicolon(it, to);

	LOG("calling evaluate_identifier_sanity()\n");

	//peek if the next token would be a function call
	if (VECTOR_PEEK(it, 1, end)) {
		LOG("is punctuation: " << it[1]->is_punctuation() << '\n');
		if (it[1]->is_punctuation()) {
			const auto token = dynamic_cast<punctuation_token_t*>(it[1].get());
			if (token->punc == P_PAR_OPEN) {
				throw compile_error("function calls are not supported yet");
			}
			else if (token->punc == P_ASSIGN) {
				return evaluate_declaration_sanity(it, end);
			}
		}
	}

	//if not, then assume this is a normal expression
	evaluate_expression_sanity(it, end);



}