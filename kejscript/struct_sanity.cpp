#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

#include "function_sanity.hpp"

static std::unique_ptr<function_block> evaluate_constructor(const std::string_view& target, struct_def* def,
	ListTokenPtr::iterator& it, ListTokenPtr::iterator& end);
static std::unique_ptr<function_block> evaluate_method(struct_def* def,
	ListTokenPtr::iterator& it, ListTokenPtr::iterator& end);
static std::unique_ptr<variable_initializer> evaluate_struct_variable_declaration(linting_data& data,
	ListTokenPtr::iterator& it, ListTokenPtr::iterator& end);

void unevaluated_struct::evaluate(ListTokenPtr::iterator& end) const
{
	auto it = block_start;

	auto& data = linting_data::getInstance();

	data.active_scope = linting_create_scope_without_range(data.active_scope);
	data.active_scope->scope_type = scope_type_e::STRUCT;
	data.active_struct = def;

	std::queue<unevaluated_function> constructor_blocks;
	std::queue<unevaluated_function> method_blocks;

	const char* ptr = 0;
	while (it != end && it->get()->is_operator(P_CURLYBRACKET_CLOSE) == false) {

		switch (it->get()->tt) {
		case tokentype_t::DEF:
			if (std::next(it) != end) {
				ptr = std::next(it)->get()->string.c_str();
			}
			else
				throw linting_error(it->get(), "unexpected eof");

			def->quick_lookup[ptr] = { static_cast<ptrdiff_t>(def->initializers.size()) };
			def->initializers.push_back(std::move(evaluate_struct_variable_declaration(data, it, end)));
			std::advance(it, 1);

			ptr = 0;

			break;
		case tokentype_t::FN:

			if (std::next(it) != end) {
				ptr = std::next(it)->get()->string.c_str();
			}
			else
				throw linting_error(it->get(), "unexpected eof");

			def->quick_method_lookup[ptr] = { static_cast<ptrdiff_t>(def->methods.size()) };

			def->methods.push_back(std::move(evaluate_method(def, it, end)));
			data.current_function = def->methods.back().get();
			data.current_function->structure = def;

			if (VECTOR_PEEK(it, 1, end) == false)
				throw linting_error(it->get(), "unexpected eof");

			method_blocks.push({ data.current_function, std::next(it) });
			it = skip_function(it, end);

			std::advance(it, 1);

			ptr = 0;


			break;

		case tokentype_t::IDENTIFIER:
			def->constructors.push_back(std::move(evaluate_constructor(def->identifier, def, it, end)));

			data.current_function = def->constructors.back().get();
			data.current_function->structure = def;

			if (VECTOR_PEEK(it, 1, end) == false)
				throw linting_error(it->get(), "unexpected eof");

			constructor_blocks.push({ data.current_function, std::next(it) });
			it = skip_function(it, end);

			std::advance(it, 1);
			break;
		default:
			throw linting_error(it->get(), "expected a variable declaration, a constructor or a function declaration");
		}

	}

	if (it == end)
		throw linting_error(std::prev(it)->get(), "why does the file end here");

	while (constructor_blocks.size()) {
		auto& front = constructor_blocks.front();
		front.evaluate(end, false);
		constructor_blocks.pop();
	}

	while (method_blocks.size()) {
		auto& front = method_blocks.front();
		front.evaluate(end, true);
		method_blocks.pop();
	}

	data.active_struct = 0;
	data.active_scope = linting_delete_scope(it, data.active_scope);

	//std::advance(it, -1);//go back to the token before the }

}

std::unique_ptr<variable_initializer> evaluate_struct_variable_declaration(linting_data& data,
	ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	auto scope = data.active_scope;

	std::advance(it, 1); //skip the def keyword

	if(it == end)
		throw linting_error("expected an identifier instead of EOF");

	if (it->get()->is_identifier() == false)
		throw linting_error(it->get(), "expected an identifier");

	if (scope->declare_variable(it->get()->string) == false)
		throw linting_error(it->get(), "the variable '%s' is already defined", it->get()->string.c_str());

	LOG("declaring: '" << it->get()->string << "'\n");

	std::unique_ptr<variable_initializer> init = std::make_unique<variable_initializer>();

	init->variable = it->get()->string;

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error(it->get(), "expected a '=' or ';'");
	}

	std::advance(it, 1);

	if (it->get()->is_operator(P_SEMICOLON)) {
		return std::move(init);
	}

	if (it->get()->is_punctuation()) {
		const auto punc = dynamic_cast<punctuation_token_t*>(it->get())->punc;

		if (punc != punctuation_e::P_ASSIGN && punc != punctuation_e::P_SEMICOLON)
			throw linting_error(it->get(), "expected a '=' or ';'");

		std::advance(it, 1); //skip the =

		if (it == end)
			throw linting_error("expected an expression instead of EOF");
	}

	init->initializer = std::make_unique<expression_block>();
	it = evaluate_expression_sanity(it, end, init->initializer).it;
	return std::move(init);

}
std::unique_ptr<function_block> evaluate_constructor(const std::string_view& target, struct_def* def,
	ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{

	if (it->get()->string != target)
		throw linting_error(it->get(), "expected '%s' instead of '%s'", target.data(), it->get()->string.c_str());

	auto result = parse_function_declaration(it, end);

	for (auto& constructor : def->constructors) {

		if (result->def.parameters.size() == constructor->def.parameters.size())
			throw linting_error(it->get(), "a constructor with %u arguments has already been defined", result->def.parameters.size());

	}

	//std::cout << result->def.identifier << '\n';

	return std::move(result);

}
std::unique_ptr<function_block> evaluate_method(struct_def* def,
	ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	std::advance(it, 1); //skip fn

	if(it == end)
		throw linting_error(std::prev(it)->get(), "expected an identifier");

	if (!it->get()->is_identifier())
		throw linting_error(it->get(), "expected an identifier");

	auto result = parse_function_declaration(it, end);

	for (auto& method : def->methods) {
		if (result->def.identifier == method->def.identifier) {
			throw linting_error(it->get(), "the method '%s' has already been defined", result->def.identifier.c_str());
		}

	}

	return std::move(result);
}
void evaluate_struct_sanity(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected an identifier instead of EOF");
	}

	std::advance(it, 1); //skip the struct keyword

	auto& data = linting_data::getInstance();
	std::unique_ptr<struct_def> _def = std::make_unique<struct_def>();



	if (it->get()->is_identifier() == false) {
		throw linting_error(std::next(it)->get(), "expected an identifier");
	}

	if (data.struct_exists(it->get()->string)) {
		throw linting_error(it->get(), "the struct '%s' is already defined");
	}

	if (VECTOR_PEEK(it, 1, end) == false) {
		throw linting_error("expected a '{' instead of EOF");
	}

	_def->identifier = it->get()->string;
	auto& def = data.struct_declare(_def);
	std::advance(it, 1); //skip the identifier keyword

	if (it->get()->is_operator(P_CURLYBRACKET_OPEN) == false) {
		throw linting_error(std::next(it)->get(), "expected a '{'");
	}

	std::advance(it, 1); //skip the the '{'

	if (it == end)
		throw linting_error(std::prev(it)->get(), "why does the file end here");

	data.unevaluated_structs.insert({ def->identifier , { def.get(), it }});

	it = skip_function(std::prev(it), end);

	return;

	

	//throw linting_error("gg");

}