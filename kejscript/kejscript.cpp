// kejscript.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.hpp"

#include "linting_evaluate.hpp"
#include "linting_scope.hpp"
#include "runtime_exception.hpp"

int main()
{
    std::cout << "Hello World!\n";

    script_t script("scripts\\script.kej");

    bool linting_failed = false;

    try {
        script.validate();

    }
    catch (linting_error& ex) {
        std::cout << ex.what() << "\n\n";
        linting_failed = true;
    }

    auto& data = linting_data::getInstance();
    
    while (data.active_scope) {
        linting_scope* temp = data.active_scope->lower_scope;
        delete data.active_scope;
        data.active_scope = temp;
    }

    if (linting_failed) {
        LOG("stopping execution\n");
        return 0;
    }

    try {
        script.execute();
    }
    catch (runtime_error& ex) {
        std::cout << ex.what() << "\n\n";
    }

    return 1;
}
