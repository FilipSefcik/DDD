#include "callbacks.hpp"
#include <iostream>

bool divide_evenly(std::vector<module_info*>* modules, int nodeCount) {
    if (modules->empty()) {
        std::cerr << "There are no modules to divide.\n";
        return false;
    }

    int nodeUsed = 0;

    for (module_info* mod : *modules) {
        mod->set_assigned_process(nodeUsed);
        nodeUsed = (nodeUsed + 1) % nodeCount;
    }

    return true;
}

bool divide_by_var_count(std::vector<module_info*>* modules, int nodeCount) {
    if (modules->empty()) {
        std::cerr << "There are no modules to divide.\n";
        return false;
    }

    std::vector<int> nodeVarCount;
    nodeVarCount.resize(nodeCount);

    std::sort(modules->begin(), modules->end(), [](module_info* a, module_info* b) {
        return a->get_var_count() > b->get_var_count();
    });

    for (module_info* mod : *modules) {
        int modVarCount = mod->get_var_count();
        int nodeUsed = 0;

        for (size_t i = 0; i < nodeVarCount.size(); i++) {
            if (nodeVarCount.at(i) == 0) {
                nodeUsed = i;
                break;
            }

            if (nodeVarCount.at(nodeUsed) > nodeVarCount.at(i)) {
                nodeUsed = i;
            }
        }
        nodeVarCount.at(nodeUsed) += modVarCount;
        mod->set_assigned_process(nodeUsed);
    }

    return true;
}

void add_instruction_density(module_info* mod, std::string* instructions) {
        module_info* parent = mod->get_parent();

        // EXEC - module name - position of the module in parent
        *instructions += "EXEC " + mod->get_name() + " " + std::to_string(mod->get_position()) + "\n";
        if (parent) {
            if (mod->get_assigned_process() == parent->get_assigned_process()) {
                // LINK - name of parent module - name of son module
                *instructions += "LINK " + parent->get_name() + " " + mod->get_name() + "\n";
            } else {
                // SEND - name of module - rank of the process to send
                *instructions += "SEND " + mod->get_name() + " " + std::to_string(parent->get_assigned_process()) + "\n";

                // RECV - parent module name - rank of the process received from
                *(instructions + 1) += "RECV " + parent->get_name() + " " + std::to_string(mod->get_assigned_process()) + "\n";
            }
        } else {
            // END - module which gives answer
            *instructions += "END " + mod->get_name() + "\n";
        }
}