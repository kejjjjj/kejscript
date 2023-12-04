#include "pch.hpp"

bool is_unary_operator(const punctuation_e p)
{
	if (p == punctuation_e::P_UNKNOWN)
		return false;

	if (p == P_INCR || p == P_DECR)
		return true;

	if (p == P_ADD || p == P_SUB)
		return true;

	return false;
}
bool bad_unary_operator(const punctuation_e p)
{
	return is_unary_operator(p) == false;
}

bool is_postfix_operator(const punctuation_e p)
{
	if (p == punctuation_e::P_UNKNOWN)
		return false;

	if (p == P_INCR || p == P_DECR)
		return true;

	if (p == P_BRACKET_OPEN)
		return true;

	if (p == P_PAR_OPEN)
		return true;

	return false;
}

bool satisfies_operator(const punctuation_e p)
{
	if (p == punctuation_e::P_UNKNOWN)
		return false;

	if (p < punctuation_e::P_PAR_OPEN)
		return true;

	return false;
}