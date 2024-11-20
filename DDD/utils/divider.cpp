#include "divider.hpp"
#include <algorithm>
#include <iostream>

/*
 * @brief Checks if modules can be divided.
 *
 * Throws an error if no modules are provided.
 */
void divider::check(std::vector<module*>* modules) {
    if (modules->empty()) {
        std::cerr << "There are no modules to divide.\n";
        exit(3);
    }
}

/*
 * @brief Divides modules among nodes evenly based on no particular property.
 */
void node_divider::divide_modules(std::vector<module*>* modules, std::vector<module*> nodes[],
                                  std::vector<int>* count) {
    this->check(modules);

    int nodeUsed = 0;

    for (module* mod : *modules) {
        nodes[nodeUsed].push_back(mod);
        count->at(nodeUsed)++;
        mod->assign_process(nodeUsed);
        nodeUsed = (nodeUsed + 1) % count->size();
    }
}

/*
 * @brief Divides modules among nodes evenly based on how many variables modules have.
 */
void var_count_divider::divide_modules(std::vector<module*>* modules, std::vector<module*> nodes[],
                                       std::vector<int>* count) {
    this->check(modules);

    std::vector<int> nodeVarCount;
    nodeVarCount.resize(count->size());

    std::sort(modules->begin(), modules->end(),
              [](module* a, module* b) { return a->get_var_count() > b->get_var_count(); });

    for (module* mod : *modules) {
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
        mod->assign_process(nodeUsed);
    }
}
