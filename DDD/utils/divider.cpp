#include "divider.hpp"
#include <algorithm>
#include <iostream>

/*
 * @brief Checks if modules can be divided.
 *
 * Throws an error if no modules are provided.
 */
void divider::check(std::vector<module_info*>* modules) {
    if (modules->empty()) {
        std::cerr << "There are no modules to divide.\n";
        exit(3);
    }
}

/*
 * @brief Divides modules among nodes evenly based on no particular property.
 */
void node_divider::divide_modules(std::vector<module_info*>* modules,
                                  std::vector<module_info*> nodes[], std::vector<int>* count) {
    this->check(modules);

    int nodeUsed = 0;

    for (module_info* mod : *modules) {
        nodes[nodeUsed].push_back(mod);
        count->at(nodeUsed)++;
        mod->set_assigned_process(nodeUsed);
        nodeUsed = (nodeUsed + 1) % count->size();
    }
}

/*
 * @brief Divides modules among nodes evenly based on how many variables modules have.
 */
void var_count_divider::divide_modules(std::vector<module_info*>* modules,
                                       std::vector<module_info*> nodes[], std::vector<int>* count) {
    this->check(modules);

    std::vector<int> nodeVarCount;
    nodeVarCount.resize(count->size());

    std::sort(modules->begin(), modules->end(), [](module_info* a, module_info* b) {
        return a->get_var_count() > b->get_var_count();
    });

    for (module_info* mod : *modules) {
        int modVarCount = mod->get_var_count();
        int nodeUsed = 0;

        for (int i = 0; i < nodeVarCount.size(); i++) {
            if (nodeVarCount.at(i) == 0) {
                nodeUsed = i;
                break;
            }

            if (nodeVarCount.at(nodeUsed) > nodeVarCount.at(i)) {
                nodeUsed = i;
            }
        }
        nodeVarCount.at(nodeUsed) += modVarCount;
        nodes[nodeUsed].push_back(mod);
        count->at(nodeUsed)++;
        mod->set_assigned_process(nodeUsed);
    }
}
