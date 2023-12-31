#pragma once

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
	std::unique_ptr<std::vector<char>> value = nullptr;

	bool is_identifier() const noexcept(true) { return tt == tokentype_t::IDENTIFIER; }
	bool is_reserved_keyword() const noexcept(true) { return tt > tokentype_t::IDENTIFIER; }
	virtual bool is_operator(const punctuation_e punctuation) const noexcept;

	virtual bool is_punctuation() const noexcept(true) { return false; }
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
	bool assign_to_stack_if_possible(ListTokenPtr::iterator& it) noexcept {

		if (time_to_exit())
			return false;

		auto token = it->get();

		if (!token->is_punctuation())
			return false;

		const auto p = dynamic_cast<const punctuation_token_t*>(token);

		if (p->punc == opening) {
			stack.num_open++;
			LOG("incrementing " << punctuations[p->punc].identifier << " to " << stack.num_open << " at " 
				<< it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");
			return true;
		}
		else if (p->punc == closing) {
			stack.num_close++;
			LOG("incrementing " << punctuations[p->punc].identifier << " to " << stack.num_close << " at "
				<< it->get()->string << " at [" << it->get()->line << ", " << it->get()->column << "]\n");

			location = it;
			return true;
		}
		return false;
	}

	bool time_to_exit() const noexcept {
		return stack.num_open && stack.num_close == stack.num_open;
	}

	bool not_in_use() const noexcept { return opening == P_UNKNOWN || closing == P_UNKNOWN; }

	token_stack stack;
	punctuation_e opening = punctuation_e::P_UNKNOWN;
	punctuation_e closing = punctuation_e::P_UNKNOWN;
	ListTokenPtr::iterator location;
	size_t num_evaluations = 0; //the number of recursive calls to evaluate expression

};
struct function_def
{
	std::vector<std::string> parameters;
	std::string identifier;
	std::vector<std::string> variables;

	
};
enum class code_block_e : uint8_t
{
	FUNCTION,
	CONDITIONAL,
	WHILE,
	FN_CALL,
	EXPRESSION,
	RETURN,
	INITIALIZER_LIST
};
struct function_block;
struct code_block
{
	explicit code_block() = default;
	virtual ~code_block() = default;
	//std::list<std::unique_ptr<code_block>> contents; maybe one day

	//if this returns true, then it means that the current function wants to return
	virtual bool execute(struct function_stack* stack) = 0; 
	bool eval_block(struct function_stack* stack);

	virtual code_block_e type() const noexcept(true) = 0;
	function_block* owner = 0; //the function that owns this code block
	std::list<std::unique_ptr<code_block>> contents;
};
struct ast_node;
struct function_block
{
	function_block(const function_def& d) : def(d){}

	template<typename t>
	void add_instruction(std::unique_ptr<t>& instruction) {
		instructions.push_back(std::move(instruction));
		blocks.push_back(instructions.back().get());
	}
	size_t get_index_for_variable(const std::string_view& target) {
		size_t i = 0;
		
		for (auto& v : def.variables) {
			if (!v.compare(target)) {
				return i;
			}
			++i;
		}
		assert("get_index_for_variable(): didn't find variable.. how?");
		return 0;
	}
	std::list<std::unique_ptr<code_block>> instructions;
	std::vector<code_block*> blocks; //keeps track of the current codeblock (points to the instructions list)
	//struct function_stack* stack = 0;
	struct operand* return_value = 0;
	size_t nest_depth = 0;
	function_def def;
	bool entrypoint = false;
	NO_COPY_CONSTRUCTOR(function_block);
};

//this stuff should be elsewhere i am pretty sure!
enum class operator_type : char
{
	UNARY,
	STANDARD,
	POSTFIX
};
struct _operator {
	OperatorPriority priority = OperatorPriority::FAILURE;
	punctuation_e punc = punctuation_e::P_UNKNOWN;
	void* eval = 0;
	operator_type type = operator_type::STANDARD;
	token_t* token = 0;
};
struct expression_block : public code_block
{
	expression_block() = default;
	~expression_block() = default;

	bool execute(struct function_stack* stack) override;

	std::unique_ptr<ast_node> ast_tree;
	std::unique_ptr<expression_block> next; //indicates that there was more than one evaluation (function call arguments or initializer lists)
	code_block_e type() const noexcept(true) override { return code_block_e::EXPRESSION; }

};
struct conditional_block : public code_block
{
	conditional_block() = default;
	~conditional_block() = default;
	std::unique_ptr<expression_block> condition;
	bool execute(struct function_stack* stack) override;

	code_block_e type() const noexcept(true) override { return code_block_e::CONDITIONAL; }
	std::unique_ptr<conditional_block> next; //jump location if the condition is not true
};

struct function_call : public code_block
{
	function_call() = default;
	~function_call() = default;
	function_block* target = 0;
	std::unique_ptr<expression_block> arguments;
	bool execute(struct function_stack* stack) override { stack; return false; }; //implement!

	code_block_e type() const noexcept(true) override { return code_block_e::FN_CALL; }
};
struct while_block : public code_block
{
	while_block() = default;
	~while_block() = default;
	std::unique_ptr<expression_block> condition; //I don't want to initialize it in the constructor just to be a bit more explicit
	bool execute(struct function_stack* stack) override;
	code_block_e type() const noexcept(true) override { return code_block_e::WHILE; }
};
struct return_statement : public code_block
{
	return_statement() = default;
	~return_statement() = default;
	bool execute(struct function_stack* stack) override;
	std::unique_ptr<expression_block> expression; //this can be a nullptr if returning void
	code_block_e type() const noexcept(true) override { return code_block_e::RETURN; }

};
struct initializer_list : public code_block
{
	initializer_list() = default;
	~initializer_list() = default;
	bool execute(struct function_stack* stack) override {};
	std::unique_ptr<initializer_list> next;
	std::unique_ptr<expression_block> expression;
	code_block_e type() const noexcept(true) override { return code_block_e::INITIALIZER_LIST; }

};
struct validation_expression
{
	enum Type
	{
		LITERAL,
		OTHER
	}type = OTHER;
	struct literal
	{
		enum literal_type
		{
			NUMBER_LITERAL,
			FLOAT_LITERAL,
			STRING_LITERAL,
			CHAR_LITERAL,
			_TRUE,
			_FALSE,
		};

		literal_type type = NUMBER_LITERAL;
		std::vector<char> value;

	};
	struct other
	{
		std::string identifier;
		size_t variable_index = 0; //for quick access

	};
	validation_expression() = default;
	validation_expression(const literal& l) : value(l), type(LITERAL) {};
	validation_expression(const other& o) : value(o), type(OTHER) {};

	void make_literal(const validation_expression::literal& e)
	{
		value = e;
		type = validation_expression::LITERAL;
	}
	void make_other(const validation_expression::other& e)
	{
		value = e;
		type = validation_expression::OTHER;
	}
	std::variant<literal, other> value;
};
struct singular {
	singular() = default;
	singular(const validation_expression::literal& e) : v(e) {}
	singular(const validation_expression::other& e) : v(e) {}

	void make_literal(const validation_expression::literal& e) { v.make_literal(e); }
	void make_other(const validation_expression::other& e){ v.make_other(e); }

	validation_expression v;
	token_t* token = nullptr;
	function_block* owner = 0; // the function that owns this operand
	std::unique_ptr<function_call> callable;

	std::unique_ptr<ast_node> parentheses;

	NO_COPY_CONSTRUCTOR(singular);
	//ListTokenPtr::iterator location;
};
using singularlist = std::list<std::unique_ptr<singular>>;
using operatorlist = std::list<_operator>;
using nodeptr = std::unique_ptr<ast_node>;

struct ast_node
{
	ast_node() = default;

	enum Type
	{
		OPERATOR,
		OPERAND,
	}type=OPERATOR;

	nodeptr left;
	nodeptr right;
	std::variant<std::unique_ptr<singular>, _operator> contents = nullptr;
	bool is_leaf() const noexcept { return type == OPERAND; }
	bool is_unary() const noexcept { return type == OPERATOR && std::get<_operator>(contents).type == operator_type::UNARY; }
	void make_operator(const _operator& op) {
		contents = op;
		type = OPERATOR;
	}
	void make_operand(std::unique_ptr<singular>& s) {
		contents = std::move(s);
		type = OPERAND;
	}

	constexpr std::unique_ptr<singular>& get_operand()
	{
		return std::get<std::unique_ptr<singular>>(contents);
	}
	constexpr _operator& get_operator()
	{
		return std::get<_operator>(contents);
	}


	//void print_tree(int depth = 0, bool newline = true) const {

	//	for (int i = 0; i < depth; ++i) {
	//		std::cout << "  ";
	//	}

	//	if (contents) {
	//		std::cout << contents->token->string << (newline ? "\n" : "");
	//	}

	//	if (left) {
	//		left->print_tree(depth - 1, false);
	//	}
	//	if (right) {
	//		right->print_tree(depth + 1, true);
	//	}
	//}

	NO_COPY_CONSTRUCTOR(ast_node);
};

struct expression_context
{
	expression_context(const expression_token_stack& _stack) : stack(_stack) {}
	expression_token_stack stack;

	std::unique_ptr<expression_block> block;
	singularlist ex;
	std::list<_operator> operators;
	std::string identifier;

	NO_COPY_CONSTRUCTOR(expression_context);
};