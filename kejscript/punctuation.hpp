#pragma once

#include "pch.hpp"

enum OperatorPriority : char
{
	FAILURE,
	ASSIGNMENT,		//	= += -= *= /= %= >>= <<= &= ^= |=
	CONDITIONAL,	//	?:
	LOGICAL_OR,		//	||
	LOGICAL_AND,	//	&&
	BITWISE_OR,		//  | 
	BITWISE_XOR,	//	^
	BITWISE_AND,	//  & 
	EQUALITY,		//	< <= > >=
	RELATIONAL,		//	== !=
	SHIFT,			//  <<>>
	ADDITIVE,		//	+-
	MULTIPLICATIVE,	//	* / %
	UNARY,			//  + - ! ~ ++ - - (type)* & sizeof	
	POSTFIX,		//  () [] -> . ++ - -	

};
enum punctuation_e : char
{

	P_ADD,
	P_SUB,

	P_LESS_THAN,
	P_GREATER_THAN,

	P_ASSIGN,

	P_EQUALITY,
	P_UNEQUALITY,
	P_MODULO,
	P_MULTIPLICATION,
	P_DIVISION,

	P_LOGICAL_AND,

	P_INCREMENT,

	P_PAR_OPEN,
	P_PAR_CLOSE,

	P_BRACKET_OPEN,
	P_BRACKET_CLOSE,

	P_CURLYBRACKET_OPEN,
	P_CURLYBRACKET_CLOSE,

	P_COMMA,
	P_PERIOD,
	P_SEMICOLON,

	P_UNKNOWN,
};

struct punctuation_t 
{
	std::string identifier;
	punctuation_e punc = P_UNKNOWN;
	OperatorPriority priority = FAILURE;
};

const static punctuation_t punctuations[] =
{
	//needs to be sorted from longest string to shortest

	{"==", P_EQUALITY, RELATIONAL },
	{"!=", P_UNEQUALITY, RELATIONAL },

	{"&&", P_LOGICAL_AND, LOGICAL_AND},

	{"++", P_INCREMENT, UNARY},
	

	{"+", P_ADD, ADDITIVE},
	{"-", P_SUB, ADDITIVE},

	{"<", P_LESS_THAN, EQUALITY},
	{">", P_GREATER_THAN, EQUALITY},

	{"%", P_MODULO, MULTIPLICATIVE},
	{"*", P_MULTIPLICATION, MULTIPLICATIVE},
	{"/", P_DIVISION, MULTIPLICATIVE},

	{"=", P_ASSIGN, ASSIGNMENT},
	
	{"(", P_PAR_OPEN, POSTFIX},
	{")", P_PAR_CLOSE, FAILURE},

	{"[", P_BRACKET_OPEN, POSTFIX},
	{"]", P_BRACKET_CLOSE, FAILURE},

	{"{", P_CURLYBRACKET_OPEN, FAILURE},
	{"}", P_CURLYBRACKET_CLOSE, FAILURE},

	{",", P_COMMA},
	{".", P_PERIOD, POSTFIX},
	{";", P_SEMICOLON }
};

bool is_unary_operator(const punctuation_e p);
bool bad_unary_operator(const punctuation_e p);

bool is_postfix_operator(const punctuation_e p);
bool satisfies_operator(const punctuation_e p);
