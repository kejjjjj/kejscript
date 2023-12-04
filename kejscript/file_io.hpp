#include "pch.hpp"

namespace fs
{
	enum class fileopen
	{
		FILE_IN = 1,
		FILE_OUT = 2,
		FILE_APP = 8,
		FILE_BINARY = 32
	};

	std::string get_exe_file_name();
	std::string get_exe_path();
	std::string get_root_directory();
	std::string go_to_previous_directory(const std::string& directory);

	bool open_file(std::fstream& fp, const std::string& path, const fileopen type);
	bool close_file(std::fstream& fp);

	bool file_exists(const std::string_view& file);
	bool is_directory(const std::string_view& file);

	std::string full_path_to_name(const std::string_view& path);
	std::string file_extension(const std::string_view& file);

}
std::string get_last_error();