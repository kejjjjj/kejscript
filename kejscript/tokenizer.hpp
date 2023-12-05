#include "pch.hpp"

enum class codeblock_parser_type : char
{
	PREPROCESSOR,
	DEFAULT,
	CREATE_SCOPE,
	DELETE_SCOPE

};

enum class tokentype_t : uint8_t
{
	UNKNOWN,
	NUMBER_LITERAL,
	FLOAT_LITERAL,
	STRING_LITERAL,
	CHAR_LITERAL,
	PUNCTUATION,
	IDENTIFIER,
	DEF,
	FN,
	RETURN
};

struct token_t
{
	token_t() = default;
	token_t(const std::string& t, tokentype_t _tt)
		: string(t), tt(_tt)
	{
		
	}
	virtual ~token_t() = default;
	std::string string;
	tokentype_t tt = tokentype_t::UNKNOWN;
	size_t line = 0;
	size_t column = 0;
	bool is_identifier() const noexcept(true) { return tt == tokentype_t::IDENTIFIER; }
	bool is_reserved_keyword() const noexcept(true) { return tt > tokentype_t::IDENTIFIER; }
	virtual bool is_operator(const punctuation_e punctuation) const noexcept;

	virtual bool is_punctuation() const noexcept(true) { return false; }

};
using VectorTokenPtr = std::vector<std::unique_ptr<token_t>>;

struct punctuation_token_t : public token_t
{
	punctuation_token_t(const punctuation_t& p) :
		token_t(p.identifier, tokentype_t::PUNCTUATION), punc(p.punc), priority(p.priority) {
	}
	~punctuation_token_t() = default;

	punctuation_e punc = P_UNKNOWN;
	OperatorPriority priority = FAILURE;

	bool is_punctuation() const noexcept(true) override { return true; }
	//a quicker implementation that doesn't need a dynamic cast
	bool is_operator(const punctuation_e punctuation) const noexcept override {
		return punc == punctuation;
	}
};
