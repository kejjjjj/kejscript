#include "pch.hpp"
#include "while_statement.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

while_block* block = 0;

bool evaluation_result(while_block* _block)
{
	auto results = evaluate_expression(_block->condition_start, _block->condition_end);

	const auto& node = *results->expression.get();
	const auto& result_expression = std::get<std::unique_ptr<operand>>(node._op);
	bool_dt condition_evaluation = datatype::create_type<bool_dt, bool>(*result_expression->get_value());

	return condition_evaluation.get();
}

void evaluate_while_statement(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end)
{
	block = dynamic_cast<while_block*>(it->get()->block.get());
	auto& stack = runtime::get_instance().stack;


	if (evaluation_result(block)) {

		stack->conditional_block = std::make_unique<stack_t::block_t>(tokentype_t::WHILE);
		stack->conditional_block->start = std::next(block->start);
		stack->conditional_block->end = block->end;
		stack->conditional_block->jmp = std::next(block->start);
		return;
	}

	it = block->end;
	end = block->end;

}



bool handle_while_conditional(ListTokenPtr::iterator& it, std::unique_ptr<stack_t::block_t>& conditional_block)
{
	
	
	//LOG("hello\n");
	if (conditional_block->start == conditional_block->end) {
		
		bool result = evaluation_result(block);

		if (result == true) {
			conditional_block->start = conditional_block->jmp;
			return 0;

		}

		it = (conditional_block->end);
		conditional_block.reset();
		return 1;
	}

	return 0;
}