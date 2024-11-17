#include "modules/module.hpp"
#include "modules/module_manager.hpp"
#include <iostream>
#include <ostream>

int main(int argc, char* argv[]) {

    module_manager manager;
    manager.load("../../DDD/load_files/modules/module_map.conf");
    manager.print_modules();
    int i = 0;
    for (module* mod : *manager.get_modules()) {
        mod->assign_process(i);
        i = ++i % 3;
    }
    manager.get_instructions(3);
    manager.print_assigned_processes();
    manager.print_separate_instructions();
    std::string instr = manager.get_instructions_for_process(0);
    std::cout << instr << std::endl;

    return 0;
}
