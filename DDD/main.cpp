#include "modules/module.hpp"
#include "processing/module_manager.hpp"
#include "utils/divider.hpp"
#include <iostream>
#include <ostream>
#include <vector>

int main(int argc, char* argv[]) {

    module_manager manager;
    manager.load("../../DDD/load_files/modules/module_map.conf");
    // manager.print_modules();
    std::vector<module*>* nodes = new std::vector<module*>[3];
    std::vector<int>* moduleCount = new std::vector<int>;
    moduleCount->resize(3);

    node_divider divider;
    divider.divide_modules(manager.get_modules(), nodes, moduleCount);

    for (int i = 0; i < 3; i++) {
        std::vector<module*> nodes_modules = nodes[i];
        std::cout << "Node " << i << std::endl;

        for (module* mod : nodes_modules) {
            mod->print_sons();
        }
        std::cout << std::endl;
    }

    manager.get_instructions(3);
    // manager.print_assigned_processes();
    // manager.print_separate_instructions();
    std::string instr = manager.get_instructions_for_process(2);
    std::cout << instr << std::endl;

    return 0;
}
