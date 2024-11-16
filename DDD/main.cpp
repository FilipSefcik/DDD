#include "modules/module.hpp"
#include <iostream>
#include <ostream>

void print_array(std::vector<double>* arr) {
    for (double prob : *arr) {
        std::cout << prob << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {

    module* mod = new module("M0", 3);
    mod->set_var_count(1);
    mod->print_sons_reliabilities();

    std::vector<double> probs = {0.5, 0.25, 0.25};
    mod->set_my_reliability(&probs);
    print_array(mod->get_my_reliabilities());
    print_array(&probs);

    mod->set_sons_reliability(0, &probs);
    probs = {0, 0, 0};
    print_array(&probs);
    print_array(mod->get_my_reliabilities());
    mod->print_sons_reliabilities();
    return 0;
}
