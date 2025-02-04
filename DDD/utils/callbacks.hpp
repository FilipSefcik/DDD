#pragma once
#include "../modules/module_info.hpp"
#include <vector>

bool divide_evenly(std::vector<module_info*>* modules, int nodeCount);
bool divide_by_var_count(std::vector<module_info*>* modules, int nodeCount);

