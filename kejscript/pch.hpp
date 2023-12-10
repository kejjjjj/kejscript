#pragma once

#define NOMINMAX

#define ENABLE_linting_LOGGING 0

#if ENABLE_linting_LOGGING
#define LOG(x) std::cout << x
#else
#define LOG(x)
#endif

#define VECTOR_PEEK(it, index, end) (std::next(it) != end)


#include <Windows.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <string_view>
#include <filesystem>
#include <direct.h>
#include <array>
#include <list>
#include <functional>
#include <unordered_map>
#include <any>
#include <numeric>
#include <source_location>
#include <unordered_set>
#include <variant>

#include "punctuation.hpp"
#include "tokenizer.hpp"
#include "script.hpp"
//#include "linting_exceptions.hpp"
//#include "linting_scope.hpp"
//#include "linting_evaluate.hpp"

#include "file_io.hpp"

namespace _fs = std::filesystem;
