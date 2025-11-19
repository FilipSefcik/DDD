#pragma once
#include "../modules/module_info.hpp"
#include "../processing/mpi_manager.hpp"
#include <vector>

int is_binary_pla(const std::string& path, int* states, std::vector<int>* domains);

bool divide_evenly(std::vector<module_info*>* modules, int nodeCount);
bool divide_by_var_count(std::vector<module_info*>* modules, int nodeCount);

//--------True density---------

void add_instruction_density(module_info* mod, std::string* instructions, int condition = 0);
void calculate_true_density(mpi_manager* manager, const std::string& inputString);
std::string serialize_true_density(mpi_manager* manager, const std::string& inputString);
void deserialize_true_density(mpi_manager* manager, const std::string& parameter,
                              const std::string& inputString);

// ----------Merging-----------

bool divide_for_merging(std::vector<module_info*>* modules, int nodeCount);
void add_instruction_merging(module_info* mod, std::string* instructions, int condition = 0);
void execute_merging(mpi_manager* manager, const std::string& inputString);
std::string serialize_merging(mpi_manager* manager, const std::string& inputString);
void deserialize_merging(mpi_manager* manager, const std::string& parameter,
                         const std::string& inputString);

// --------Logical derivatives---------

void add_instruction_derivatives(module_info* mod, std::string* instructions, int condition = 0);
std::string serialize_derivatives(mpi_manager* manager, const std::string& inputString);
void deserialize_derivatives(mpi_manager* manager, const std::string& parameter,
                             const std::string& inputString);