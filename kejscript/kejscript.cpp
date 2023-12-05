// kejscript.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"

int main()
{
    std::cout << "Hello World!\n";


    try {
        script_t script("scripts\\script.kej");

        script.validate();

    }
    catch (linting_error& ex) {
        std::cout << ex.what() << "\n\n";
    }

    auto& data = linting_data::getInstance();
    
    while (data.active_scope) {
        linting_scope* temp = data.active_scope->lower_scope;
        delete data.active_scope;
        data.active_scope = temp;
    }

}
