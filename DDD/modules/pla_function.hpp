#pragma once
#include <string>

class pla_function {
  private:
    char** variables_ = nullptr;
    char* fun_values_ = nullptr;
    int fun_val_count_[2] = {0, 0}; // [0] = 0s, [1] = 1s
    int num_lines_ = 0;
    int var_count_ = 0;

  public:
    // Konštruktory a destruktor.
    pla_function() = default;
    pla_function(int varCount, int lineCount);
    pla_function(const std::string& filePath) {
        this->load_from_pla(filePath);
    }
    ~pla_function();

    void assign(const pla_function& other);

    // Gettery.
    char** get_variables() const { return this->variables_; };
    char* get_function_values() const { return this->fun_values_; };
    int get_num_lines() const { return this->num_lines_; };
    int get_var_count() const { return this->var_count_; };
    const int* get_fun_val_count() const { return this->fun_val_count_; };

    void load_from_pla(const std::string& filePath);
    void write_to_pla(const std::string& filePath) const;

    // Funkcie na alokáciu a uvoľnenie hodnôt.
    void alloc_values();
    void free_values();

    // Metódy na manipuláciu s funkciou.
    void add_line(const char* newVars, char value, int lineNum);
    void replace_char(const char* before, int beforeLength, int position, const char* input,
                      int inputLength, char* result);
    void free_sort(char*** sorted, int groupCount);
    char*** sort_by_function();
    char*** sort_by_position(int position, int* matchCount);
    void input_variables(pla_function* other, int position);
    char get_fun_value(const char* indexedVariables);

    void print_function() const;
};