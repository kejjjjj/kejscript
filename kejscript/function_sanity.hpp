#pragma once

#include "tokenizer.hpp"

ListTokenPtr::iterator skip_function(ListTokenPtr::iterator it, ListTokenPtr::iterator& end);

struct unevaluated_function
{
	function_block* target = 0;
	ListTokenPtr::iterator block_start;

	void evaluate(ListTokenPtr::iterator& end, bool returning_allowed = true) const;

};

struct unevaluated_struct
{
	struct_def* def = 0;
	ListTokenPtr::iterator block_start;

	void evaluate(ListTokenPtr::iterator& end) const;

};