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
    this->modules_ = new std::vector<module*>();
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
    auto constexpr isSpace = [](auto const character) {
        return static_cast<bool>(std::isspace(character));
    };

    auto file = std::ifstream(confPath);

    if (! file.is_open()) {
        throw std::runtime_error("Error opening conf file");
        return;
    }

    // Read paths to modules and create all modules
    std::string name, path, column, line;
    std::unordered_map<std::string, int> moduleMapping;

    while (std::getline(file, line)) {
        if (line[0] == '#' || line.empty()) {
            continue;
        }

        int spaceIndex = line.find(" ");
        int plaIndex = line.find(".pla");

        if (plaIndex == std::string::npos) {
            break;
        }

        name = line.substr(0, spaceIndex);
        path = line.substr(spaceIndex + 1, plaIndex + 3 - name.length());
        column = line.substr(plaIndex + 5);

        std::cout << "Je potrebne zadat ako nacitat stavy. Ci to dat do conf suboru alebo to "
                     "vycitat z pla.\n";
        module* mod = new module(name, 2);
        mod->set_path(path);
        mod->set_function_column(std::stoi(column));

        moduleMapping.emplace(mod->get_name(), this->modules_->size());
        this->modules_->push_back(mod);
    }

    // Read mapping and connect modules between each other
    do {
        auto const first = std::find_if_not(std::begin(line), std::end(line), isSpace);
        auto const last = std::end(line);
        if (first == last || *first == '#') {
            // Skip empty line.
            continue;
        }

        // key == module name
        // val == mapping of modules and variables for this module

        auto const keyLast = std::find_if(first, last, isSpace);
        auto const valFirst = keyLast == last ? last : std::find_if_not(keyLast + 1, last, isSpace);
        auto key = std::string(first, keyLast);
        if (valFirst != last) {
            auto valLast = last;
            while (isSpace(*(valLast - 1))) {
                --valLast;
            }
            auto val = std::string(valFirst, valLast);

            int digits = 0;
            int position;
            std::stringstream moduleName;
            for (std::size_t i = 0; i < val.size(); ++i) {
                if (val[i] == 'M') {
                    moduleName.str("");
                    moduleName << 'M';
                    position = i - digits;

                    for (size_t j = i + 1; j < val.size(); j++) {
                        if (val[j] == 'V' || val[j] == 'M') {
                            i = j - 1;
                            break;
                        }
                        moduleName << val[j];
                        digits++;
                    }
                    this->modules_->at(moduleMapping.at(moduleName.str()))->set_position(position);
                    this->modules_->at(moduleMapping.at(key))
                        ->add_son(this->modules_->at(moduleMapping.at(moduleName.str())));
                    this->modules_->at(moduleMapping.at(key))
                        ->add_son_position(moduleName.str(), position);
                }
            }
            this->modules_->at(moduleMapping.at(key))->set_var_count(val.size() - digits);
        }

    } while (std::getline(file, line));
}

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
              [](module* a, module* b) { return a->get_priority() < b->get_priority(); });

    for (int i = 0; i < this->modules_->size(); i++) {
        module* mod = this->modules_->at(i);
        module* parent = mod->get_parent();
        if (! this->separate_instructions_->at(mod->get_process_rank())) {
            this->separate_instructions_->at(mod->get_process_rank()) = new std::stringstream;
        }

        // EXEC - module name - position of the module in parent
        *this->separate_instructions_->at(mod->get_process_rank())
            << "EXEC " << mod->get_name() << " " << mod->get_position() << "\n";
        if (parent) {
            if (! this->separate_instructions_->at(parent->get_process_rank())) {
                this->separate_instructions_->at(parent->get_process_rank()) =
                    new std::stringstream;
            }

            if (mod->get_process_rank() == parent->get_process_rank()) {
                // LINK - name of parent module - name of son module
                *this->separate_instructions_->at(mod->get_process_rank())
                    << "LINK " << parent->get_name() << " " << mod->get_name() << "\n";
            } else {
                // SEND - name of module - rank of the process to send
                *this->separate_instructions_->at(mod->get_process_rank())
                    << "SEND " << mod->get_name() << " " << parent->get_process_rank() << "\n";

                // RECV - parent module name - rank of the process received from
                *this->separate_instructions_->at(parent->get_process_rank())
                    << "RECV " << parent->get_name() << " " << mod->get_process_rank() << "\n";
            }
        } else {
            // END - module which gives answer
            *this->separate_instructions_->at(mod->get_process_rank())
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

    for (module* mod : *this->modules_) {
        std::cout << mod->get_name() << " " << mod->get_path() << "\n";
        std::cout << "My position: " << mod->get_position() << "\n";
        mod->print_sons();
        std::cout << "\n";
    }
}

void module_manager::print_assigned_processes() {
    if (this->modules_->empty()) {
        return;
    }

    for (module* mod : *this->modules_) {
        std::cout << mod->get_name() << " " << mod->get_process_rank() << "\n";
    }
}

void module_manager::print_separate_instructions() {
    for (int i = 0; i < this->separate_instructions_->size(); i++) {
        std::cout << "Node " << i << " instructions:\n";
        std::cout << this->separate_instructions_->at(i)->str() << "\n";
    }
}
