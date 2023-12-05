#include "pch.hpp"

bool token_t::is_operator(const punctuation_e punctuation) const noexcept {
	return is_punctuation() && dynamic_cast<const punctuation_token_t*>(this)->punc == punctuation;
}