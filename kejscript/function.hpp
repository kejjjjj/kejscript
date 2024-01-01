#pragma once

#include "pch.hpp"
#include "operand.hpp"

std::unique_ptr<operand> call_function(function_block* caller, function_block* callee, expression_block* args);
