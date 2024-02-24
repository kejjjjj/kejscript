#include "pch.hpp"
#include "linting_exceptions.hpp"


bool token_t::is_operator(const punctuation_e punctuation) const noexcept {
	return is_punctuation() && dynamic_cast<const punctuation_token_t*>(this)->punc == punctuation;
}
bool code_block::eval_block(function_stack* stack) {

	for (auto& instruction : contents) {
		if (instruction->execute(stack))
			return true;

	}

	return false;

}

size_t function_block::get_index_for_variable(const std::string_view& target)
{
	size_t i = 0;

	if (structure) {
		for (auto& v : structure->initializers) {
			if (!v->variable.compare(target)) {
				return i;
			}
			++i;
		}
	}

	i = 0;
	for (auto& v : def.variables) {
		if (!v.compare(target)) {
			return i;
		}
		++i;
	}
	assert("get_index_for_variable(): didn't find variable.. how?");
	return 0;
}