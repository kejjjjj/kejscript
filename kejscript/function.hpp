#pragma once

#include "pch.hpp"

void transfer_code_execution(VectorTokenPtr::iterator& it, VectorTokenPtr::iterator& end, const VectorTokenPtr::iterator& return_loc, const function_def& func);