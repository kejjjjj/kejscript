#include "pch.hpp"
#include "linting_exceptions.hpp"
#include "linting_evaluate.hpp"

#include "runtime.hpp"

script_t::script_t(const std::string& filename)
{
	std::string path = filename;
	std::fstream f;
	path = fs::get_root_directory() + "\\" + path;
	//std::cout << "path: " << path << '\n';
	this->filename = filename;
	if (!fs::open_file(f, path, fs::fileopen::FILE_IN)) {
		return;
	}
	//std::cout << std::format("\"{}\" successfully opened!\n", path);
	char buf[4096];
	while (f.read(buf, sizeof(buf)))
		this->buffer.append(buf, sizeof(buf));
	this->buffer.append(buf, f.gcount());

	fs::close_file(f);

	if (this->buffer.empty()) {
		return;
	}

	this->script_p = this->buffer.begin();
	this->lastscript_p = script_p;
	this->scriptend_p = this->buffer.end();
	this->line = 1ull;
	this->column = 1ull;

	while (auto token = read_token()) {
		tokens.push_back(std::move(token));
	}

	token_it = tokens.begin();

}
std::unique_ptr<token_t> script_t::read_token()
{
	lastscript_p = script_p;
	
	std::unique_ptr<token_t> token(new token_t);

	if (!read_whitespace())
		return nullptr;

	token->line = line;
	token->column = column;

	if (std::isdigit(*script_p) || *script_p == '.' && std::isdigit(*(script_p + 1))) {
		if (!read_number(*token)) {
			return nullptr;
		}
	}
	else if (std::isalpha(*script_p) || *script_p == '_') {
		if (!read_name(*token)) {
			return nullptr;
		}
	}
	else if (*script_p == '\'') {
		if (!read_characterliteral(*token)) {
			return nullptr;
		}
	}
	else if (*script_p == '\"') {
		if (!read_string_literal(*token)) {
			return nullptr;
		}
	}
	else {
		if (!read_punctuation(token)) {
			throw linting_error(this, "a token without a definition");
		}
	}

	return std::move(token);
}
bool script_t::read_whitespace()
{
	if (script_p == scriptend_p)
		return 0;

	while (*script_p <= ' ') {


		if (script_p == scriptend_p)
			return 0;

		if (*script_p == '\n') {
			line++;
			column = 1ull;
		}
		else {
			if (*script_p == '\t')
				column += 4;
			else
				column++;
		}

		script_p++;

		if (script_p == scriptend_p)
			return 0;
	}

	return 1;
}
bool script_t::read_number(token_t& token)
{
	//a floating point literal with . prefix
	if (*script_p == '.') { //assumes that there is a number next

		//if the character after the dot is not a number, then stop
		if (!std::isdigit(*(script_p + 1))) {
			return 0;
		}

		token.string.push_back(*script_p++);
		token.tt = tokentype_t::FLOAT_LITERAL;

		//parse the integer literal after the .
		if (!parse_int(token))
			return 0;

		std::vector<char> v(sizeof(double));
		std::from_chars(token.string.c_str(), token.string.c_str() + token.string.size(), reinterpret_cast<double&>(*v.data()));
		token.value = std::make_unique<std::vector<char>>(v);

	}

	else if (std::isdigit(*script_p)) {
		token.tt = tokentype_t::NUMBER_LITERAL;

		if (!parse_int(token))
			return 0;

		if (script_p == scriptend_p)
			return 1;

		//floating point decimal
		if (*script_p == '.') {
			token.string.push_back(*script_p++);
			token.tt = tokentype_t::FLOAT_LITERAL;

			//parse the integer literal after the .
			if (!parse_int(token))
				return 0;

			std::vector<char> v(sizeof(double));
			std::from_chars(token.string.c_str(), token.string.c_str() + token.string.size(), reinterpret_cast<double&>(*v.data()));
			token.value = std::make_unique<std::vector<char>>(v);
		}
		//an integer
		else {
			std::vector<char> v(sizeof(int32_t));
			std::from_chars(token.string.c_str(), token.string.c_str() + token.string.size(), reinterpret_cast<int32_t&>(*v.data()));
			token.value = std::make_unique<std::vector<char>>(v);
		}
		//todo -> suffixes
	}

	column += token.string.length();
	return 1;

}
const std::unordered_map<std::string, tokentype_t> tokenMap = {
	{"def",		tokentype_t::DEF},
	{"fn",		tokentype_t::FN},
	{"return",  tokentype_t::RETURN},
	{"if",		tokentype_t::IF},
	{"else",	tokentype_t::ELSE},
	{"while",	tokentype_t::WHILE},
	{"for",		tokentype_t::FOR},
	{"true",	tokentype_t::_TRUE},
	{"false",	tokentype_t::_FALSE},
	{"struct",	tokentype_t::STRUCT}
};


bool script_t::read_name(token_t& token)
{
	token.tt = tokentype_t::IDENTIFIER;
	token.string.push_back(*script_p++);

	if (script_p == scriptend_p)
		return 1;

	while (std::isalnum(*script_p) || *script_p == '_') {

		token.string.push_back(*script_p++);

		if (script_p == scriptend_p)
			break;

	}

	const auto reserved_keyword = tokenMap.find(token.string);
	if (reserved_keyword != tokenMap.end()) {
		token.tt = reserved_keyword->second;


		if (token.tt == tokentype_t::_FALSE || token.tt == tokentype_t::_TRUE) {
			std::vector<char> v(1);
			if (token.tt == tokentype_t::_TRUE)			v[0] = 1;
			else	v[0] = 0;

			token.value = std::make_unique<std::vector<char>>(v);
		}
	}

	column += token.string.length();
	return 1;
}
bool script_t::read_characterliteral(token_t& token)
{
	token.tt = tokentype_t::CHAR_LITERAL;
	script_p++;

	if (script_p == scriptend_p)
		return 0;

	while (*script_p != '\'') {

		if (script_p == scriptend_p)
			return 0;

		if (*script_p == '\n') {
			line++;
			column = 1ull;
		}
		else {
			if (*script_p == '\t')
				column += 4;
			else
				column++;
		}

		token.string.push_back(*script_p++);

	}
	script_p++;
	column++;

	std::vector<char> v;

	for (auto& c : token.string)
		v.push_back(c);

	token.value = std::make_unique<std::vector<char>>(v);

	return 1;
}
bool script_t::read_string_literal(token_t& token)
{
	token.tt = tokentype_t::STRING_LITERAL;
	script_p++; //skip "

	if (script_p == scriptend_p)
		return 0;

	while (*script_p != '\"') {

		if (script_p == scriptend_p)
			return 0;

		if (*script_p == '\n') {
			line++;
			column = 1ull;
		}
		else {
			if (*script_p == '\t')
				column += 4;
			else
				column++;
		}

		token.string.push_back(*script_p++);

	}
	script_p++;  //skip "
	column++;

	std::vector<char> v;

	for (auto& c : token.string)
		v.push_back(c);

	token.value = std::make_unique<std::vector<char>>(v);

	return 1;
}
bool script_t::read_punctuation(std::unique_ptr<token_t>& token)
{
	for (const auto& i : punctuations) {

		//check eof
		if((size_t)std::distance(script_p, scriptend_p) >= i.identifier.length()){

			auto end = script_p + i.identifier.length();

			const std::string punctuation = std::string(script_p, end);

			//is the punctuation valid?
			if (!punctuation.compare(i.identifier)) {
				const size_t _line = token->line;
				const size_t _column = token->column;

				token = std::unique_ptr<punctuation_token_t>(new punctuation_token_t(i));
				token->line = _line;
				token->column = _column;

				script_p += token->string.length();
				column += token->string.length();
				return 1;
			}
		}
	}
	return 0;
}

bool script_t::parse_int(token_t& token)
{
	if (script_p == scriptend_p)
		return 0;

	if (std::isdigit(*script_p)) {
		token.string.push_back(*script_p++);

		if (script_p == scriptend_p)
			return 1;

		while (isdigit(*script_p)) {
			token.string.push_back(*script_p++);

			if (script_p == scriptend_p)
				return 1;

		}
	}

	return 1;
}

void script_t::validate()
{
	auto& data = linting_data::getInstance();
	data.tokens = &tokens;
	data.validate(tokens.begin(), tokens.end());

}
void script_t::execute()
{
	runtime::initialize(linting_data::getInstance().function_table);
	runtime::sorted_literals = linting_data::getInstance().sorted_literals;
	runtime::execute();


}

ListTokenPtr::iterator seek_block_end(ListTokenPtr::iterator it, const ListTokenPtr::iterator& end)
{
	size_t num_open = 0;
	size_t num_close = 0;

	while (it != end) {

		if (it->get()->is_operator(P_CURLYBRACKET_OPEN))
			num_open++;
		else if (it->get()->is_operator(P_CURLYBRACKET_CLOSE))
			num_close++;

		if (num_open == num_close)
			return it;

		std::advance(it, 1);
	}

	return it;

}