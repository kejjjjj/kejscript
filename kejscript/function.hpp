#pragma once

#include "pch.hpp"

void transfer_code_execution(ListTokenPtr::iterator& it, ListTokenPtr::iterator& end, const ListTokenPtr::iterator& return_loc, const function_def& func);