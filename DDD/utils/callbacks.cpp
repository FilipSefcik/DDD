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