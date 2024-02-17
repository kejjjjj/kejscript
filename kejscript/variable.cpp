#include "variable.hpp"
#include "operand.hpp"


void variable::insert_element(struct operand* val)
{
	std::unique_ptr<variable> var = std::make_unique<variable>();

	switch (val->type) {
	case operand::Type::LVALUE:
		var = std::unique_ptr<variable>(std::get<variable*>(val->value));
		break;
	case operand::Type::RVALUE:
		var->value = std::move(std::get<datatype_ptr>(val->value));
		break;	
	case operand::Type::RVALUE_ARRAY:
		auto& a = std::get<std::vector<operand_ptr>>(val->value);
		for (auto& o : a) {
			var->insert_element(o.get());
		}

		break;
	}

	arrayElements.push_back(std::move(var));
}

void variable::print(size_t spaces)
{
	if (!spaces)
		std::cout << identifier << '\n';
	
	std::string prefix = "  ";
	for (size_t i = 0; i < spaces; i++) {
		prefix += "  ";
	}
	size_t i = 0;
	for (auto& arr : arrayElements) {

		if (arr->arrayElements.size()) {
			std::cout << std::format("{}[{}]: \n", prefix, i);
		}
		++i;

		arr->print(spaces + 1);
	}


	if (arrayElements.empty()) {
		auto& v = std::get<std::unique_ptr<datatype>>(value);
		std::cout << std::format("{}| <{}> ({})\n", prefix, v->type_str(), v->value_str());
	}
	


}