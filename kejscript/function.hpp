#pragma once

#include "pch.hpp"
#include "operand.hpp"

std::unique_ptr<operand> call_function(function_block* caller, function_block* callee, const std::list<std::unique_ptr<operand>>& args, struct function_stack* stack, struct function_stack** main_stack=0);
