#include "callbacks.hpp"
#include "libteddy/inc/reliability.hpp"
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <libteddy/inc/io.hpp>
#include <ostream>
#include <sstream>

bool divide_evenly(std::vector<module_info*>* modules, int nodeCount) {
    if (modules->empty()) {
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

bool divide_for_merging(std::vector<module_info*>* modules, int nodeCount) {
    if (modules->empty()) {
        return false;
    }

    std::sort(modules->begin(), modules->end(),
              [](module_info* a, module_info* b) { return a->get_priority() < b->get_priority(); });

    int nodeUsed = 0;

    for (module_info* mod : *modules) {
        if (mod->get_son_count() == 0) {
            mod->set_assigned_process(mod->get_parent()->get_assigned_process());
            continue;
        }
        mod->set_assigned_process(nodeUsed);
        nodeUsed = (nodeUsed + 1) % nodeCount;
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
            *instructions += "SEND " + mod->get_name() + " " +
                             std::to_string(parent->get_assigned_process()) + "\n";

            // RECV - parent module name - rank of the process received from
            *(instructions + 1) += "RECV " + parent->get_name() + " " +
                                   std::to_string(mod->get_assigned_process()) + "\n";
        }
    } else {
        // END - module which gives answer
        *instructions += "END " + mod->get_name() + "\n";
    }
}

void add_instruction_merging(module_info* mod, std::string* instructions) {
    module_info* parent = mod->get_parent();

    if (parent) {
        if (parent->get_assigned_process() == mod->get_assigned_process()) {
            // LINK - name of parent module - name of son module
            *instructions += "LINK " + parent->get_name() + " " + mod->get_name() + "\n";
        } else {
            // SEND - name of module - rank of the process to send
            *instructions += "SEND " + mod->get_name() + " " +
                             std::to_string(parent->get_assigned_process()) + "\n";

            // RECV - parent module name - rank of the process received from
            *(instructions + 1) += "RECV " + parent->get_name() + " " +
                                   std::to_string(mod->get_assigned_process()) + "\n";
        }

        std::string latest_path = mod->get_latest_path();

    } else {
        // END - module which gives answer
        *instructions += "FIN " + mod->get_name() + "\n";
    }
}

void calculate_true_density(mpi_manager* manager, const std::string& inputString) {
    std::string keyWord, paramFirst, paramSecond;
    std::istringstream inputStream(inputString);
    inputStream >> keyWord >> paramFirst;

    if (keyWord == "EXEC") {
        inputStream >> paramSecond;
        module* mod = manager->get_my_modules().at(paramFirst);
        if (mod) {
            mod->set_position(std::stoi(paramSecond));
            std::string const& path = mod->get_path();
            int pla_type = is_binary_pla(path, nullptr, nullptr);
            std::vector<double> ps;
            if (pla_type == 1) {
                std::optional<teddy::pla_file_binary> file = teddy::load_binary_pla(path, nullptr);
                teddy::bss_manager bssManager(file->input_count_, mod->get_var_count() * 100);
                teddy::bdd_manager::diagram_t f =
                    teddy::io::from_pla(bssManager, *file)[mod->get_function_column()];
                ps = bssManager.calculate_probabilities(*mod->get_sons_reliability(), f);
            } else if (pla_type == 0) {
                std::optional<teddy::pla_file_mvl> file = teddy::load_mvl_pla(path, nullptr);
                // mod->set_sons_reliability(&file->domains_);
                // std::cout << std::endl;
                // std::cout << file->codomain_ << std::endl;
                // std::cout << std::endl;
                teddy::imss_manager imssManager(file->input_count_, mod->get_var_count() * 100,
                                                file->domains_);
                teddy::imss_manager::diagram_t f = teddy::io::from_pla(imssManager, *file);
                ps = imssManager.calculate_probabilities(*mod->get_sons_reliability(), f);
            } else {
                std::cout << "Invalid PLA file.\n";
                return;
            }

            // mod->print_sons_reliabilities();

            mod->set_my_reliability(&ps);

            mod->print_reliabilities();

        } else {
            std::cout << "Module not found.\n";
        }
    } else if (keyWord == "LINK") {
        inputStream >> paramSecond;
        module* parent = manager->get_my_modules().at(paramFirst);
        module* son = manager->get_my_modules().at(paramSecond);
        if (parent && son) {
            parent->set_sons_reliability(son->get_position(), son->get_my_reliabilities());
        } else {
            std::cout << "No module found.\n";
        }
    } else if (keyWord == "END") {
        module* mod = manager->get_my_modules().at(paramFirst);
        if (mod) {
            int state = manager->get_calculated_state();
            if (state < 0 || state >= mod->get_states()) {
                std::cout << "Invalid state\n";
                return;
            }
            std::cout << "Density of " << state << ": " << mod->get_reliability(state) << std::endl;
        } else {
            std::cout << "Module not found.\n";
        }
    }
}

std::string serialize_true_density(module* mod) {
    std::string result = std::to_string(mod->get_position());
    for (double rel : *mod->get_my_reliabilities()) {
        result += " " + std::to_string(rel);
    }
    return result;
}

void deserialize_true_density(const std::string& inputString, module* mod) {
    std::istringstream line(inputString);
    int sonPosition;
    line >> sonPosition;
    std::vector<double> sonRels;
    double temp;
    while (line >> temp) {
        sonRels.push_back(temp);
    }
    mod->set_sons_reliability(sonPosition, &sonRels);
}

int is_binary_pla(const std::string& path, int* states, std::vector<int>* domains) {
    std::ifstream file(path);
    if (! file.is_open()) {
        std::cerr << "Chyba pri otváraní súboru: " << path << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line)) {
        // Ignorujeme komentáre
        if (! line.empty() && line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token; // Prvé slovo (napr. .i alebo .mv)

        // Ak riadok začína ".i", znamená to binárnu funkciu
        if (token == ".i") {
            if (domains) {
                int number;
                while (iss >> number) {
                    domains->push_back(number);
                }
            }
            if (states) {
                *states = 2; // Binárna funkcia má vždy 2 stavy
            }
            return 1; // Binárna funkcia
        }

        // Ak riadok začína ".mv", znamená to viachodnotovú funkciu
        if (token == ".mv") {
            int number;
            std::vector<int> numbers;
            while (iss >> number) {
                numbers.push_back(number);
            }
            if (! numbers.empty()) {
                if (states) {
                    *states = numbers.back(); // Posledné číslo ide do `states`
                }
                if (domains) {
                    numbers.erase(numbers.begin());
                    domains->insert(domains->end(), numbers.begin(), numbers.end());
                }
            }
            return 0; // Viachodnotová funkcia
        }
    }

    return -1; // Ak súbor neobsahuje .i ani .mv
}