#pragma once
#include "../modules/module_info.hpp"
#include "../modules/module.hpp"
#include "../processing/mpi_manager.hpp"
#include <vector>

bool divide_evenly(std::vector<module_info*>* modules, int nodeCount);
bool divide_by_var_count(std::vector<module_info*>* modules, int nodeCount);

void add_instruction_density(module_info* mod, std::string* instructions);

void calculate_true_density(mpi_manager* manager, std::string inputString);

std::string serialize_true_density(module* mod);
void deserialize_true_density(std::string inputString, module* mod);