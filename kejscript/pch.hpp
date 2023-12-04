#pragma once

#define NOMINMAX

#define ENABLE_COMPILER_LOGGING 1

#if ENABLE_COMPILER_LOGGING
#define LOG(x) std::cout << x
#else
#define LOG(x)
#endif

#define VECTOR_PEEK(it, index, end) (it + index != end)


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
#include "punctuation.hpp"

#include "tokenizer.hpp"
#include "script.hpp"
#include "exceptions.hpp"
#include "compiler_scope.hpp"
#include "compiler_evaluate.hpp"

#include "file_io.hpp"

namespace _fs = std::filesystem;
