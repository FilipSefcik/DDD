#include "pla_function.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

// Konštruktory a destruktor.
pla_function::pla_function(int varCount, int lineCount) {
    this->var_count_ = varCount;
    this->num_lines_ = lineCount;
    this->fun_val_count_[0] = 0;
    this->fun_val_count_[1] = 0;
    this->alloc_values();
}

pla_function::~pla_function() {
    this->free_values();
    this->variables_ = nullptr;
    this->fun_values_ = nullptr;
    this->num_lines_ = 0;
    this->var_count_ = 0;
}

void pla_function::assign(const pla_function& other) {
    this->free_values();
    this->num_lines_ = other.get_num_lines();
    this->var_count_ = other.get_var_count();
    this->fun_val_count_[0] = other.get_fun_val_count()[0];
    this->fun_val_count_[1] = other.get_fun_val_count()[1];
    this->variables_ = (char**)malloc(this->num_lines_ * sizeof(char*));
    for (int i = 0; i < this->num_lines_; i++) {
        this->variables_[i] = (char*)malloc(this->var_count_ * sizeof(char));
        memcpy(this->variables_[i], other.get_variables()[i], this->var_count_ * sizeof(char));
    }

    this->fun_values_ = (char*)malloc(this->num_lines_ * sizeof(char));
    memcpy(this->fun_values_, other.get_function_values(), this->num_lines_ * sizeof(char));
}

void pla_function::load_from_pla(const std::string& filePath) {

    std::ifstream inputFile(filePath);
    if (! inputFile.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath + "\n");
    }

    std::string line;
    int varCount = 0;
    int lineNum = 0;

    while (std::getline(inputFile, line)) {
        // Preskoč prázdne riadky, komentáre a príkazy začínajúce bodkou
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line[0] == '.') {
            std::istringstream iss(line);
            std::string token;
            iss >> token; // token obsahuje napríklad ".i", ".o" alebo ".p"
            if (token == ".i") {
                iss >> varCount; // načíta počet vstupných premenných
            } else if (token == ".o") {
                continue; // načíta počet výstupov (preskočíme, nepoužívame)
            } else if (token == ".p") {
                iss >> lineNum; // načíta počet riadkov logických súčinov
            } else {
                break; // ak token nie je ani ".i", ".o" ani ".p", ukončíme spracovanie
            }
            continue;
        }

        break;
    }

    if (varCount == 0 || lineNum == 0) {
        inputFile.close();
        throw std::runtime_error("Could not load relevant data from file: " + filePath + "\n");
    }

    this->free_values();
    this->num_lines_ = lineNum;
    this->var_count_ = varCount;
    this->fun_val_count_[0] = 0;
    this->fun_val_count_[1] = 0;
    this->alloc_values();

    int lineIndex = 0;
    std::string variables;
    int value;
    do {
        // Preskoč prázdne riadky, komentáre a príkazy začínajúce bodkou
        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == ".e") {
            break; // koniec súboru
        }

        std::istringstream iss(line);
        iss >> variables >> value; // načítaj premenné

        // std::cout << lineIndex << " " << variables << " " << value << std::endl;

        // Pridaj načítaný riadok do PLA funkcie
        this->add_line(variables.c_str(), value, lineIndex);
        lineIndex++;
    } while (std::getline(inputFile, line));

    inputFile.close();
}

void pla_function::write_to_pla(const std::string& filePath) const {
    std::ofstream outputFile(filePath);
    if (! outputFile.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath + "\n");
    }

    outputFile << ".i " << this->var_count_ << "\n";
    outputFile << ".o 1\n";
    outputFile << ".p " << this->num_lines_ << "\n";

    for (int i = 0; i < this->num_lines_; i++) {
        outputFile << std::string(this->variables_[i], this->var_count_) << " "
                   << this->fun_values_[i] << "\n";
    }

    outputFile << ".e\n";
    outputFile.close();
}

// Funkcie na alokáciu a uvoľnenie hodnôt.
void pla_function::alloc_values() {
    this->variables_ = (char**)malloc(this->num_lines_ * sizeof(char*));
    for (int i = 0; i < this->num_lines_; i++) {
        this->variables_[i] = (char*)malloc(this->var_count_ * sizeof(char));
    }
    this->fun_values_ = (char*)malloc(this->num_lines_ * sizeof(char));
}

void pla_function::free_values() {
    for (int i = 0; i < this->num_lines_; i++) {
        free(this->variables_[i]);
    }
    free(this->variables_);
    free(this->fun_values_);
}

// Metódy na manipuláciu s funkciou.
void pla_function::add_line(const char* newVars, char value, int lineNum) {
    memcpy(*(this->variables_ + lineNum), newVars, this->var_count_ * sizeof(char));
    *(this->fun_values_ + lineNum) = value;
    this->fun_val_count_[value - '0']++;
}

void pla_function::replace_char(const char* before, int beforeLength, int position,
                                const char* input, int inputLength, char* result) {
    if (position == 0) {
        memcpy(result, input, inputLength);
        memcpy(result + inputLength, before + 1, beforeLength - 1);
    } else {
        memcpy(result, before, position);
        memcpy(result + position, input, inputLength);
        memcpy(result + position + inputLength, before + position + 1, beforeLength - position - 1);
    }
}

void pla_function::free_sort(char*** sorted, int groupCount) {
    if (sorted) {
        for (int i = 0; i < groupCount; i++) {
            free(sorted[i]);
        }
    }
    free(sorted);
}

char*** pla_function::sort_by_function() {
    char*** sorted = (char***)malloc(2 * sizeof(char**));

    for (int i = 0; i < 2; i++) {
        *(sorted + i) = (char**)malloc(*(this->fun_val_count_ + i) * sizeof(char*));
    }

    char** varPtrCurr = this->variables_;
    char** varPtrEnd = this->variables_ + this->num_lines_;
    char* funPtrCurr = this->fun_values_;
    char* funPtrEnd = this->fun_values_ + this->num_lines_;
    char** sorted0Curr = *sorted;
    char** sorted1Curr = *(sorted + 1);

    while (varPtrCurr != varPtrEnd && funPtrCurr != funPtrEnd) {
        if (*funPtrCurr == '0') {
            *sorted0Curr++ = *varPtrCurr;
        } else if (*funPtrCurr == '1') {
            *sorted1Curr++ = *varPtrCurr;
        }
        varPtrCurr++;
        funPtrCurr++;
    }

    return sorted;
}

char*** pla_function::sort_by_position(int position, int* matchCount) {
    if (! matchCount) {
        return nullptr;
    }
    char*** sorted = (char***)malloc(3 * sizeof(char**));

    char** varPtr = this->variables_;
    for (int i = 0; i < this->num_lines_; i++, varPtr++) {
        char currentChar = *(*varPtr + position);
        if (currentChar == '0') {
            matchCount[0]++;
        } else if (currentChar == '1') {
            matchCount[1]++;
        } else if (currentChar == '-') {
            matchCount[2]++;
        }
    }

    for (int group = 0; group < 3; group++) {
        sorted[group] = (char**)malloc(matchCount[group] * sizeof(char*));
    }

    varPtr = this->variables_;
    int indexes[3] = {0, 0, 0};
    for (int i = 0; i < this->num_lines_; i++, varPtr++) {
        char* currentString = *varPtr;
        char current_char = *(currentString + position);
        if (current_char == '0') {
            sorted[0][indexes[0]++] = currentString;
        } else if (current_char == '1') {
            sorted[1][indexes[1]++] = currentString;
        } else if (current_char == '-') {
            sorted[2][indexes[2]++] = currentString;
        }
    }

    return sorted;
}

char pla_function::get_fun_value(const char* indexedVariables) {
    char** varPtr = this->variables_;
    char* funPtr = this->fun_values_;

    for (int i = 0; i < this->num_lines_; i++, varPtr++, funPtr++) {
        if (memcmp(*varPtr, indexedVariables, this->var_count_) == 0) {
            return *funPtr;
        }
    }

    return '0';
}

void pla_function::print_function() const {
    char** varPtr = this->variables_;
    char* funPtr = this->fun_values_;
    for (int i = 0; i < this->num_lines_; i++, varPtr++, funPtr++) {
        printf("%.*s\t%c\n", this->var_count_, *varPtr, *funPtr);
    }
    printf("\n");
}

void pla_function::input_variables(pla_function* other, int position) {
    if (other->get_var_count() <= 0) {
        return;
    }
    int* matchCount = (int*)calloc(3, sizeof(int));
    char*** myVars = this->sort_by_position(position, matchCount);
    char*** additionalVars = other->sort_by_function();

    int otherVarCount = other->get_var_count();
    const int* otherFunValCount = other->get_fun_val_count();

    char* whateverInput = new char[otherVarCount];
    memset(whateverInput, '-', otherVarCount * sizeof(char));

    int newVarCount = otherVarCount + this->var_count_ - 1;
    int newLineCount =
        matchCount[0] * otherFunValCount[0] + matchCount[1] * otherFunValCount[1] + matchCount[2];
    char* newVars = new char[newVarCount];

    pla_function newPla(newVarCount, newLineCount);

    int lineNum = 0;
    for (int group = 0; group < 3; group++) {
        for (int i = 0; i < matchCount[group]; i++) {
            char* tempLine = myVars[group][i];
            char funValue = this->get_fun_value(tempLine);
            if (group < 2) {
                for (int j = 0; j < otherFunValCount[group]; j++) {
                    char* inputLine = additionalVars[group][j];
                    this->replace_char(tempLine, this->var_count_, position, inputLine,
                                       otherVarCount, newVars);
                    newPla.add_line(newVars, funValue, lineNum);
                    lineNum++;
                }
            } else {
                this->replace_char(tempLine, this->var_count_, position, whateverInput,
                                   otherVarCount, newVars);
                newPla.add_line(newVars, funValue, lineNum);
                lineNum++;
            }
        }
    }
    std::cout << "BEFORE\n";
    this->print_function();
    std::cout << "AFTER\n";
    newPla.print_function();

    this->assign(newPla);

    newPla.~pla_function();
    free(matchCount);
    delete[] whateverInput;
    delete[] newVars;

    this->free_sort(myVars, 3);
    this->free_sort(additionalVars, 2);

    // this->print_function();
}
