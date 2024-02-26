#pragma once

#include "pch.hpp"
#include "operators.hpp"

[[maybe_unused]] operand* evaluate_expression(function_block* owner, struct function_stack* stack, const nodeptr& node);
