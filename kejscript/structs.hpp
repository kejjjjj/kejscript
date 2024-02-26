#pragma once

#include "pch.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

std::unique_ptr<operand> call_constructor(
	struct_call* callee, 
	const std::list<operand*>& args, 
	struct function_stack* stack);

std::unique_ptr<operand> call_method(
	function_block* callee,
	const std::list<operand*>& args,
	std::shared_ptr<object>& _this);
