#pragma once

#include "pch.hpp"
#include "operators.hpp"

[[maybe_unused]] std::unique_ptr<operand> evaluate_expression(const nodeptr& node);
