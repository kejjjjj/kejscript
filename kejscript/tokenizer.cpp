#include "pch.hpp"
#include "linting_exceptions.hpp"


bool token_t::is_operator(const punctuation_e punctuation) const noexcept {
	return is_punctuation() && dynamic_cast<const punctuation_token_t*>(this)->punc == punctuation;
}
bool code_block::eval_block(function_stack* stack) {

	for (auto& instruction : contents) {
		if (instruction->execute(stack))
			return true;

	}

	return false;

}

size_t function_block::get_index_for_variable(const std::string_view& target)
{
	size_t i = 0;

	if (structure) {
		for (auto& v : structure->initializers) {
			if (!v->variable.compare(target)) {
				return i;
			}
			++i;
		}
	}

	i = 0;
	for (auto& v : def.variables) {
		if (!v.compare(target)) {
			return i;
		}
		++i;
	}
	return 0;
	//throw linting_error("get_index_for_variable(): didn't find '%s'", target.data());
}
size_t function_block::get_index_for_operand(const std::string& target)
{
	return def.operands.find(target)->second;
}
void ast_node::print_internal(int level, std::vector<std::vector<std::string>>& levels) const
{
	if (!this)
		return;

	if (!initialized)
		throw linting_error("uninitialized node");

	std::string a = (type == Type::OPERAND
		? std::get<std::unique_ptr<singular>>(contents)->token->string :
		std::get<operator_ptr>(contents)->token->string);

	if (levels.size() <= level)
		levels.resize(size_t(level + 1));

	levels[level].push_back(a);

	left->print_internal(level + 1, levels);
	right->print_internal(level + 1, levels);
}