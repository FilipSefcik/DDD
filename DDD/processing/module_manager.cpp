#include "module_manager.hpp"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

module_manager::module_manager() {
    this->modules_ = new std::vector<module_info*>();
    this->separate_instructions_ = new std::vector<std::stringstream*>();
}

module_manager::~module_manager() {
    for (int i = 0; i < this->modules_->size(); i++) {
        delete this->modules_->at(i);
    }

    for (int i = 0; i < this->separate_instructions_->size(); i++) {
        delete this->separate_instructions_->at(i);
    }

    this->modules_->clear();
    this->separate_instructions_->clear();

    delete modules_;
    delete separate_instructions_;
}

void module_manager::load(std::string confPath) {
    try {
        this->load_modules(confPath);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
}

void module_manager::load_modules(std::string confPath) {
    auto constexpr is_space = [](char character) {
        return std::isspace(static_cast<unsigned char>(character));
    };

    std::ifstream file(confPath);
    if (! file.is_open()) {
        throw std::runtime_error("Error opening conf file");
    }

    std::unordered_map<std::string, int> moduleMapping;
    std::string line;

    // 1. Fáza: Načítanie ciest k modulom
    while (std::getline(file, line)) {
        // Preskoč prázdne riadky alebo komentáre
        if (line.empty() || line[0] == '#') {
            continue;
        }

        size_t plaIndex = line.find(".pla");
        if (plaIndex == std::string::npos) {
            break; // Ukončenie načítania modulov, prechádzame na vzťahy
        }

        // Extrakcia informácií o module
        size_t spaceIndex = line.find(' ');
        std::string name = line.substr(0, spaceIndex);
        std::string path = line.substr(spaceIndex + 1, plaIndex + 4 - spaceIndex - 1);
        int column = std::stoi(line.substr(plaIndex + 5));

        std::cout << "Nezabudnut ako nacitat stavy ci z pla alebo configuraku.\n";
        // Vytvorenie nového modulu
        module_info* mod = new module_info;
        mod->set_name(name);
        mod->set_states(2); // Prednastavený počet stavov
        mod->set_pla_path(path);
        mod->set_function_column(column);

        // Uloženie modulu
        moduleMapping[name] = this->modules_->size();
        this->modules_->push_back(mod);
    }

    // 2. Fáza: Načítanie vzťahov medzi modulmi
    do {
        // Preskoč prázdne riadky alebo komentáre
        if (line.empty() || line[0] == '#') {
            continue;
        }

        // Extrakcia názvu rodiča a vzťahov
        size_t spaceIndex = line.find(' ');
        std::string key = line.substr(0, spaceIndex);
        std::string val = line.substr(spaceIndex + 1);

        auto parentModule = this->modules_->at(moduleMapping.at(key));

        // Spracovanie vzťahov (M a V)
        size_t i = 0;
        while (i < val.size()) {
            if (val[i] == 'M') {
                // Ak ide o modul, extrahuj názov
                std::string moduleName = "M";
                size_t start = ++i;
                while (i < val.size() && std::isdigit(val[i])) {
                    i++;
                }
                moduleName += val.substr(start, i - start);

                // Pridaj syna so stavmi modulu
                auto childModule = this->modules_->at(moduleMapping.at(moduleName));
                parentModule->add_son(childModule->get_states());
            } else if (val[i] == 'V') {
                // Ak ide o premennú, pridaj syna so stavmi rodiča
                parentModule->add_son(parentModule->get_states());
                i++;
            } else {
                throw std::runtime_error("Unexpected character in mapping: " +
                                         std::string(1, val[i]));
            }
        }
    } while (std::getline(file, line));
}

// void module_manager::load_modules(std::string confPath) {
//     auto constexpr isSpace = [](auto const character) {
//         return static_cast<bool>(std::isspace(character));
//     };

//     auto file = std::ifstream(confPath);

//     if (! file.is_open()) {
//         throw std::runtime_error("Error opening conf file");
//         return;
//     }

//     // Read paths to modules and create all modules
//     std::string name, path, column, line;
//     std::unordered_map<std::string, int> moduleMapping;

//     while (std::getline(file, line)) {
//         if (line[0] == '#' || line.empty()) {
//             continue;
//         }

//         int spaceIndex = line.find(" ");
//         int plaIndex = line.find(".pla");

//         if (plaIndex == std::string::npos) {
//             break;
//         }

//         name = line.substr(0, spaceIndex);
//         path = line.substr(spaceIndex + 1, plaIndex + 3 - name.length());
//         column = line.substr(plaIndex + 5);

//         std::cout << "Je potrebne zadat ako nacitat stavy. Ci to dat do conf suboru alebo to "
//                      "vycitat z pla.\n";
//         module_info* mod = new module_info;
//         mod->set_name(name);
//         mod->set_states(2);
//         mod->set_pla_path(path);
//         mod->set_function_column(std::stoi(column));

//         moduleMapping.emplace(mod->get_name(), this->modules_->size());
//         this->modules_->push_back(mod);
//     }

//     // Read mapping and connect modules between each other
//     do {
//         auto const first = std::find_if_not(std::begin(line), std::end(line), isSpace);
//         auto const last = std::end(line);
//         if (first == last || *first == '#') {
//             // Skip empty line.
//             continue;
//         }

//         // key == module name
//         // val == mapping of modules and variables for this module

//         auto const keyLast = std::find_if(first, last, isSpace);
//         auto const valFirst = keyLast == last ? last : std::find_if_not(keyLast + 1, last,
//         isSpace); auto key = std::string(first, keyLast); if (valFirst != last) {
//             auto valLast = last;
//             while (isSpace(*(valLast - 1))) {
//                 --valLast;
//             }
//             auto val = std::string(valFirst, valLast);

//             int digits = 0;
//             int position;
//             std::stringstream moduleName;
//             moduleName.str("a");
//             for (std::size_t i = 0;
//                  /*i < val.size() &&*/ i + moduleName.str().size() - 1 < val.size(); ++i) {
//                 if (val[i] == 'M') {
//                     moduleName.str("");
//                     moduleName << 'M';
//                     position = i - digits;

//                     for (size_t j = i + 1; j < val.size(); j++) {
//                         if (val[j] == 'V' || val[j] == 'M') {
//                             i = j - 1;
//                             break;
//                         }
//                         moduleName << val[j];
//                         digits++;
//                     }
//                     this->modules_->at(moduleMapping.at(moduleName.str()))->set_position(position);
//                     this->modules_->at(moduleMapping.at(key))
//                         ->add_son(
//                             this->modules_->at(moduleMapping.at(moduleName.str()))->get_states());
//                     // this->modules_->at(moduleMapping.at(key))
//                     //     ->add_son_position(moduleName.str(), position);
//                 } else {
//                     // temporary solution
//                     this->modules_->at(moduleMapping.at(key))
//                         ->add_son(this->modules_->at(moduleMapping.at(key))->get_states());
//                 }
//             }
//             // this->modules_->at(moduleMapping.at(key))->set_var_count(val.size() - digits);
//         }

//     } while (std::getline(file, line));
// }

/*
 * @brief Creates instructions on how to process all modules.
 * @param processCount Number of all processes.
 *
 * Creates separate instructions for each process.
 * Instructions consist of: Executing module, Sending module, Receiving module, Link modules, End
 * of processing.
 */
void module_manager::get_instructions(int processCount) {
    this->separate_instructions_->resize(
        processCount > this->modules_->size() ? this->modules_->size() : processCount);

    std::sort(this->modules_->begin(), this->modules_->end(),
              [](module_info* a, module_info* b) { return a->get_priority() < b->get_priority(); });

    for (int i = 0; i < this->modules_->size(); i++) {
        module_info* mod = this->modules_->at(i);
        module_info* parent = mod->get_parent();
        if (! this->separate_instructions_->at(mod->get_assigned_process())) {
            this->separate_instructions_->at(mod->get_assigned_process()) = new std::stringstream;
        }

        // EXEC - module name - position of the module in parent
        *this->separate_instructions_->at(mod->get_assigned_process())
            << "EXEC " << mod->get_name() << " " << mod->get_position() << "\n";
        if (parent) {
            if (! this->separate_instructions_->at(parent->get_assigned_process())) {
                this->separate_instructions_->at(parent->get_assigned_process()) =
                    new std::stringstream;
            }

            if (mod->get_assigned_process() == parent->get_assigned_process()) {
                // LINK - name of parent module - name of son module
                *this->separate_instructions_->at(mod->get_assigned_process())
                    << "LINK " << parent->get_name() << " " << mod->get_name() << "\n";
            } else {
                // SEND - name of module - rank of the process to send
                *this->separate_instructions_->at(mod->get_assigned_process())
                    << "SEND " << mod->get_name() << " " << parent->get_assigned_process() << "\n";

                // RECV - parent module name - rank of the process received from
                *this->separate_instructions_->at(parent->get_assigned_process())
                    << "RECV " << parent->get_name() << " " << mod->get_assigned_process() << "\n";
            }
        } else {
            // END - module which gives answer
            *this->separate_instructions_->at(mod->get_assigned_process())
                << "END " << mod->get_name() << "\n";
        }
    }
}

/*
 * @brief Returns instructions assigned to a specific process.
 * @param processRank Rank of the process we want instructions for.
 * @return Instructions as a string.
 */
std::string module_manager::get_instructions_for_process(int processRank) {
    if (processRank >= 0 && processRank < this->separate_instructions_->size()) {
        return this->separate_instructions_->at(processRank)->str();
    }
    return "INVALID RANK";
}

void module_manager::print_modules() {
    if (this->modules_->empty()) {
        return;
    }

    for (module_info* mod : *this->modules_) {
        std::cout << mod->get_name() << " " << mod->get_pla_path() << "\n";
        std::cout << "My position: " << mod->get_position() << "\n";
        mod->print_sons();
        std::cout << "\n";
    }
}

void module_manager::print_assigned_processes() {
    if (this->modules_->empty()) {
        return;
    }

    for (module_info* mod : *this->modules_) {
        std::cout << mod->get_name() << " " << mod->get_assigned_process() << "\n";
    }
}

void module_manager::print_separate_instructions() {
    for (int i = 0; i < this->separate_instructions_->size(); i++) {
        std::cout << "Node " << i << " instructions:\n";
        std::cout << this->separate_instructions_->at(i)->str() << "\n";
    }
}
