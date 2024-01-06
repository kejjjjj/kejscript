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