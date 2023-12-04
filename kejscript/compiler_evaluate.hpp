#pragma once

#include "pch.hpp"

struct compiler_data
{
	static compiler_data& getInstance() { static compiler_data d; return d; }
	compiler_scope* active_scope = 0;

};

codeblock_parser_type get_codeblock_type(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);

VectorTokenPtr::iterator splice_substr_at_semicolon(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end);
void evaluate_identifier_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);

void evaluate_declaration_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);
void evaluate_expression_sanity(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& to);