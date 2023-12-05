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
	RETURN,
	IF,
	ELSE
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

struct token_stack
{
	token_stack() = default;
	token_stack(size_t o, size_t c) : num_open(o), num_close(c) {}
	size_t num_open = 0;
	size_t num_close = 0;
};

//this structure will stop parsing the expression based on stacked tokens
struct expression_token_stack
{
	expression_token_stack() = default;
	expression_token_stack(punctuation_e open_punc, punctuation_e close_punc) : opening(open_punc), closing(close_punc), stack(1, 0) {}
	void assign_to_stack_if_possible(VectorTokenPtr::iterator& it) noexcept {

		if (time_to_exit())
			return;

		auto token = it->get();

		if (!token->is_punctuation())
			return;

		const auto p = dynamic_cast<const punctuation_token_t*>(token);

		if (p->punc == opening) {
			stack.num_open++;
		}
		else if (p->punc == closing) {
			stack.num_close++;
			location = it;
		}
	}

	bool time_to_exit() const noexcept {
		return stack.num_open && stack.num_close == stack.num_open;
	}

	bool not_in_use() const noexcept { return opening == P_UNKNOWN || closing == P_UNKNOWN; }

	token_stack stack;
	punctuation_e opening = punctuation_e::P_UNKNOWN;
	punctuation_e closing = punctuation_e::P_UNKNOWN;
	VectorTokenPtr::iterator location;
	size_t num_evaluations = 0;

};