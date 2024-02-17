#pragma once

#define NOMINMAX

#define ENABLE_LOGGING 1

#if ENABLE_LOGGING
#define LOG(x) std::cout << x
#else
#define LOG(x)
#endif

#define VECTOR_PEEK(it, index, end) (std::next(it) != end)

#define NO_COPY_CONSTRUCTOR(CLASS) const CLASS& operator=(const CLASS&) = delete; \
CLASS(const CLASS&) = delete


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
#include <cassert>

#include "punctuation.hpp"
#include "tokenizer.hpp"
#include "script.hpp"


#include "file_io.hpp"

namespace _fs = std::filesystem;
