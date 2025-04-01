#pragma once
#include "../modules/module.hpp"
#include "../modules/module_info.hpp"
#include "../processing/mpi_manager.hpp"
#include <vector>

bool divide_evenly(std::vector<module_info*>* modules, int nodeCount);
bool divide_by_var_count(std::vector<module_info*>* modules, int nodeCount);
bool divide_for_merging(std::vector<module_info*>* modules, int nodeCount);

void add_instruction_density(module_info* mod, std::string* instructions);
void add_instruction_merging(module_info* mod, std::string* instructions);

void calculate_true_density(mpi_manager* manager, const std::string& inputString);

std::string serialize_true_density(module* mod);
void deserialize_true_density(const std::string& inputString, module* mod);

int is_binary_pla(const std::string& path, int* states, std::vector<int>* domains);