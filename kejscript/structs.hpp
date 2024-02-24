#pragma once

#include "pch.hpp"
#include "runtime_expression.hpp"
#include "runtime_exception.hpp"
#include "runtime.hpp"

std::unique_ptr<operand> call_constructor(
	struct_call* callee, 
	const std::list<std::unique_ptr<operand>>& args, 
	struct function_stack* stack);
