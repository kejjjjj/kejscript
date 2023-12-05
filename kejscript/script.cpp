#include "pch.hpp"

script_t::script_t(const std::string& filename)
{
	std::string path = filename;
	std::fstream f;
	path = fs::get_root_directory() + "\\" + path;
	std::cout << "path: " << path << '\n';
	this->filename = filename;
	if (!fs::open_file(f, path, fs::fileopen::FILE_IN)) {
		throw compile_error("failed to open the input file\n%s", get_last_error().c_str());
	}
	std::cout << std::format("\"{}\" successfully opened!\n", path);
	char buf[4096];
	while (f.read(buf, sizeof(buf)))
		this->buffer.append(buf, sizeof(buf));
	this->buffer.append(buf, f.gcount());

	fs::close_file(f);

	if (this->buffer.empty()) {
		throw compile_error("empty script file");
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
	else {
		if (!read_punctuation(token)) {
			throw compile_error(this, "a token without a definition");
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
		}
		//todo -> suffixes
	}

	column += token.string.length();
	return 1;

}
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

	if (token.string == "def")
		token.tt = tokentype_t::DEF;
	else if (token.string == "fn")
		token.tt = tokentype_t::FN;
	else if (token.string == "return")
		token.tt = tokentype_t::RETURN;

	column += token.string.length();
	return 1;
}
bool script_t::read_characterliteral(token_t& token)
{
	token.tt = tokentype_t::CHAR_LITERAL;
	token.string.push_back(*script_p++);

	while (*script_p != '\'') {

		if (script_p == scriptend_p)
			return 0;

		token.string.push_back(*script_p++);

	}
	token.string.push_back(*script_p++);
	column += token.string.length();
	return 1;
}
bool script_t::read_punctuation(std::unique_ptr<token_t>& token)
{
	for (const auto& i : punctuations) {

		//check eof
		if ((script_p + i.identifier.length() - 1ull) != scriptend_p) {

			const std::string punctuation = std::string(script_p, (script_p + i.identifier.length()));

			//is the punctuation valid?
			if (!punctuation.compare(i.identifier)) {
				const size_t _line = token->line;
				const size_t _column = token->column;

				token = std::make_unique<punctuation_token_t>(punctuation_token_t(i));
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

void script_t::compile()
{
	auto& data = compiler_data::getInstance();

	data.active_scope = new compiler_scope;

	auto& codepos = token_it;
	auto end = tokens.end();

	while (codepos != end) {		
		const auto parser_required = get_codeblock_type(codepos, end);

		switch (parser_required) {
		case codeblock_parser_type::CREATE_SCOPE:
			data.active_scope = compiler_create_scope_without_range(data.active_scope);
			break;
		case codeblock_parser_type::DELETE_SCOPE:
			data.active_scope = compiler_delete_scope(*this, data.active_scope);
			break;
		default:
			evaluate_identifier_sanity(codepos, end);
			break;
		}

		if (codepos == end)
			throw compile_error((--end++)->get(), "this is a temporary error but it happened because an unexpected eof was encountered!");

		++codepos;
	}

	if (!data.active_scope->is_global_scope())
		throw compile_error((--tokens.end())->get(), "expected to find a '}'");

}