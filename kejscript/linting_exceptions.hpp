#pragma once

#include "pch.hpp"

struct linting_error
{
	template<typename ... Args>
	linting_error(const token_t* _token, const std::string& reason, const Args&&... args) : token(_token) {
		char buffer[512];
		std::snprintf(buffer, sizeof(buffer), reason.c_str(), args...);
		errReason = buffer;
		errReason = "\n****** syntax error ******\n" + errReason + "\non line: [" + std::to_string(token->line) + ", " + std::to_string(token->column) + "]";
	};

	template<typename ... Args>
	linting_error(const script_t* _script, const std::string& reason, const Args&&... args) : script(_script) {
		char buffer[512];
		std::snprintf(buffer, sizeof(buffer), reason.c_str(), args...);
		errReason = buffer;
		errReason = "\n****** syntax error ******\n" + errReason + "\non line: [" + std::to_string(script->line) + ", " + std::to_string(script->column) + "]";
	};

	template<typename ... Args>
	linting_error(const std::string& reason, const Args&&... args) : errReason(reason) {
		char buffer[512];
		std::snprintf(buffer, sizeof(buffer), reason.c_str(), args...);
		errReason = buffer;
	};
	linting_error() = delete;

	[[nodiscard]] const char* const what() const noexcept {
		return errReason.empty() ? "null exception" : errReason.c_str();
	}

private:
	std::string errReason;
	const script_t* script = 0;
	const token_t* token = 0;
};