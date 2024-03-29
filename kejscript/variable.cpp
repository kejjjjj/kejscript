#include "variable.hpp"
#include "operand.hpp"



void variable::print(size_t spaces)
{
	if (!spaces)
		std::cout << identifier << '\n';
	
	std::string prefix = "  ";
	for (size_t i = 0; i < spaces; i++) {
		prefix += "  ";
	}
	if (obj) {
		size_t i = 0;
		for (auto& arr : obj->variables) {

			if (arr->obj) {
				std::cout << std::format("{}[{}]: \n", prefix, i);
			}
			++i;

			arr->print(spaces + 1);
		}
	}
	else if (string)
	{
		std::cout << std::format("{}| <{}> ({}) @ 0x{:x}\n", prefix, "string", string->get_string(), ptrdiff_t(this));
		return;
	}

	if (!obj) {

		if (value.data) {
			auto& v = value;
			std::cout << std::format("{}| <{}> ({}) @ 0x{:x}\n", prefix, v.type_str(), v.value_str(), ptrdiff_t(this));
		}else
			std::cout << std::format("{}| empty\n", prefix);

	}
	


}
void variable::print2() const
{
	print_internal();
	std::cout << " : " << identifier << '\n';
}
void variable::print_internal(bool b_is_member) const
{
	if (obj) {

		std::cout << "[ ";
		
		for (auto& arr : obj->variables) {

			bool m = (arr == obj->variables.back());
			arr->print_internal(m ? false : true);
		}

		std::cout << (b_is_member ? " ], " : " ]");
		return;
	}
	 

	const std::string postfix = b_is_member ? ", " : "";

	if (function_pointer) {
		std::cout << "fptr (" << function_pointer->def.identifier << ')' << postfix;
		return;
	}

	if (string)
	{
		std::cout << std::quoted(string->get_string()) << postfix;
		return;
	}


	if (value.data){
		std::cout << value.value_str() << postfix;
	}
	else
		std::cout << "NULL" << postfix;



}

bool variable::valueless_type() const noexcept
{
	return obj != nullptr || function_pointer != nullptr;
}