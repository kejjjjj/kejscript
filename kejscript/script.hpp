#pragma once

#include "pch.hpp"


struct script_t
{
	script_t(const std::string& filename);

	std::string filename;
	std::string buffer;
	std::string::iterator script_p;
	std::string::iterator lastscript_p;
	std::string::iterator scriptend_p;
	size_t line = 0;
	size_t column = 0;

	void validate();
	void execute();
	ListTokenPtr::iterator& get_code_pos() { return token_it; }

private:

	std::unique_ptr<token_t> read_token();
	bool read_whitespace();
	bool read_number(token_t& token);
	bool read_name(token_t& token);
	bool read_punctuation(std::unique_ptr<token_t>& token);
	bool read_characterliteral(token_t& token);
	bool read_string_literal(token_t& token);

	bool parse_int(token_t& token);


	ListTokenPtr tokens; //every token in this script file
	decltype(tokens)::iterator token_it; //current position of the iterator

	script_t(const script_t&) = delete;
	script_t& operator=(const script_t&) = delete;
};

//assumes "it" is the '{' keyword
ListTokenPtr::iterator seek_block_end(ListTokenPtr::iterator it, const ListTokenPtr::iterator& end);
