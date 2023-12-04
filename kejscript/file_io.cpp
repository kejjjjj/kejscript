#include "pch.hpp"

std::string fs::get_exe_file_name()
{
	char buffer[MAX_PATH];
	GetModuleFileName(NULL, buffer, MAX_PATH);
	return std::string(buffer);
}
std::string fs::get_exe_path()
{
	std::string f = get_exe_file_name();
	return f.substr(0, f.find_last_of("\\/"));
}
std::string fs::get_root_directory()
{
	std::string ExeDirectory = get_exe_path();

	std::string prev = go_to_previous_directory(ExeDirectory);

	return go_to_previous_directory(prev);
}
std::string fs::go_to_previous_directory(const std::string& directory)
{
	size_t pos = directory.find_last_of('\\');
	if (pos < 1)
		return directory;

	return directory.substr(0, pos);
}
bool fs::file_exists(const std::string_view& file)
{
	return _fs::exists(file);
}
bool fs::is_directory(const std::string_view& file)
{
	return _fs::is_directory(file);
}
std::string fs::full_path_to_name(const std::string_view& path)
{
	const auto p = path.find_last_of('\\');

	return std::string(path.substr(p + 1));
}
std::string fs::file_extension(const std::string_view& file)
{
	auto ext = file.find_last_of('.');

	if (ext == std::string::npos)
		return "(no extension)";

	return std::string(file.substr(ext + 1));
}

bool fs::open_file(std::fstream& fp, const std::string& path, const fileopen type)
{
	if (fp.is_open())
		return true;

	fp.open(path, static_cast<std::_Iosb<void>::_Openmode>(static_cast<int>(type)));

	if (!fp.is_open())
		return false;


	return true;
}
bool fs::close_file(std::fstream& fp)
{
	if (fp.is_open())
		fp.close();

	return true;
}

std::string get_last_error()
{
	const DWORD errorMessageID = ::GetLastError();
	CHAR* messageBuffer = nullptr;

	size_t size = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (CHAR*)&messageBuffer, 0, NULL);

	std::string output = std::string(messageBuffer, size);

	LocalFree(messageBuffer);
	return output;
}
