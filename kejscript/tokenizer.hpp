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
	_TRUE,
	_FALSE,
	PUNCTUATION,
	IDENTIFIER,
	DEF,
	FN,
	RETURN,
	IF,
	ELSE,
	WHILE
};

struct code_block;


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
	std::unique_ptr<code_block> block = nullptr;

	token_t& operator=(const token_t&) = delete;
	token_t(const token_t&) = delete;

};
using VectorTokenPtr = std::vector<std::unique_ptr<token_t>>;
using ListTokenPtr = std::list<std::unique_ptr<token_t>>;

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

	punctuation_token_t& operator=(const punctuation_token_t&) = delete;
	punctuation_token_t(const punctuation_token_t&) = delete;
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
	void assign_to_stack_if_possible(ListTokenPtr::iterator& it) noexcept {

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
	ListTokenPtr::iterator location;
	size_t num_evaluations = 0;

};

struct function_def
{
	ListTokenPtr::iterator start;
	ListTokenPtr::iterator end;
	std::vector<std::string> parameters;
	std::string identifier;
};
enum class code_block_e : uint8_t
{
	IF,
	ELSE,
	FN_CALL
};
struct code_block
{
	explicit code_block(token_t* target) : target_token(target){}
	virtual ~code_block() = default;
	virtual code_block_e type() const noexcept(true) = 0;
	token_t* target_token = nullptr;
	ListTokenPtr::iterator start;
	ListTokenPtr::iterator end;
};

struct if_block : public code_block
{
	if_block(token_t*t) : code_block(t){}
	~if_block() = default;
	ListTokenPtr::iterator condition_start;
	ListTokenPtr::iterator condition_end;
	code_block_e type() const noexcept(true) override { return code_block_e::IF; }
	std::unique_ptr<code_block> next; //jump location if the condition is not true
};
struct else_block : public code_block
{
	else_block(token_t* t) : code_block(t) {}
	~else_block() = default;
	code_block_e type() const noexcept(true) override { return code_block_e::ELSE; }
};

struct function_call : public code_block
{
	function_call(token_t* t) : code_block(t) {}
	~function_call() = default;

	struct block {
		ListTokenPtr::iterator start;
		ListTokenPtr::iterator end;
	};
	std::list<block> arguments;
	function_def* target = nullptr;

	code_block_e type() const noexcept(true) override { return code_block_e::FN_CALL; }
};
struct while_block : public code_block
{
	while_block(token_t* t) : code_block(t) {}
	~while_block() = default;
	ListTokenPtr::iterator condition_start;
	ListTokenPtr::iterator condition_end;
	code_block_e type() const noexcept(true) override { return code_block_e::ELSE; }
};
