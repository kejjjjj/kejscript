#include "if_statement.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

//void evaluate_if_statement(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, if_block* block)
//{
//	auto results = evaluate_expression(block->condition_start, block->condition_end);
//
//	auto& stack = runtime::get_instance().stack;
//
//	const auto& node = *results->expression.get();
//	const auto& result_expression = std::get<std::unique_ptr<operand>>(node._op);
//	bool_dt condition_evaluation = datatype::create_type<bool_dt, bool>(*result_expression->get_value());
//
//	if (condition_evaluation.get() == true) {
//		
//		stack->conditional_block = std::make_unique<stack_t::block_t>(tokentype_t::IF);
//		stack->conditional_block->start = std::next(block->start);
//		stack->conditional_block->end = block->end;
//		stack->conditional_block->exit = false;
//
//		code_block* block_itr = block;
//
//		//find last block
//		while (block->next) {
//
//			block_itr = block->next.get();
//
//			if (block_itr->type() == code_block_e::IF) {
//				block = dynamic_cast<if_block*>(block->next.get());
//				block_itr = block;
//			}
//			else {
//				block_itr = dynamic_cast<else_block*>(block->next.get());
//				break;
//			}
//		}
//
//		stack->conditional_block->jmp = block_itr->end;
//
//		LOG("evaluated to true\n");
//
//
//		return;
//	}
//
//	if (block->next) {
//		if (block->next->type() == code_block_e::IF) {
//			return evaluate_if_statement(it, end, dynamic_cast<if_block*>(block->next.get()));
//		}
//		else if (block->next->type() == code_block_e::ELSE) {
//			return evaluate_else_statement(it, end, dynamic_cast<else_block*>(block->next.get()));
//
//		}
//	}
//
//	//last block
//	//executes when no condition is true
//	it = block->end;
//	end = block->end;
//	//stack->conditional_block.reset();
//	LOG("hmm?");
//	//throw runtime_error(it->get(), "condition evaluated to false");
//
//}
//
//void evaluate_else_statement(ListTokenPtr::iterator& it, [[maybe_unused]]ListTokenPtr::iterator& end, else_block* block)
//{
//	it = std::prev(block->start);
//}
//
//bool handle_if_conditional(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, std::unique_ptr<stack_t::block_t>& conditional_block)
//{
//	//did we reach the end of the block?
//	if (conditional_block->start == conditional_block->end) {
//
//		//is the iterator invalid?
//		if (conditional_block->jmp._Getcont() != end._Getcont() || conditional_block->jmp == end) {
//			it = std::next(conditional_block->end);
//		}
//		else
//			it = std::next(conditional_block->jmp);
//
//		conditional_block.reset();
//
//		if (it == end)
//			return 1;
//
//		return 1;
//		
//	}
//
//	return 0;
//}