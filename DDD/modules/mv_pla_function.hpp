#pragma once
#include <string>
#include <vector>

// Multivalued PLA function (espresso-mv style, one-hot encoded).
// In memory the variables stay in substitution order; write_to_pla() puts the
// binary ones first, as the .mv format requires.
class mv_pla_function {
  public:
    struct variable {
        bool binary; // true: one char '0' '1' '-'; false: one-hot block of `states` chars
        int states; // 2 for binary variables
    };

    mv_pla_function() = default;
    mv_pla_function(std::vector<variable> vars, int outStates);
    explicit mv_pla_function(const std::string& filePath) { load_from_pla(filePath); }

    // Gettery (input variables only, the output is described by out_states).
    const std::vector<variable>& get_variables() const { return vars_; }
    int get_var_count() const { return (int)vars_.size(); }
    int get_out_states() const { return out_states_; }
    int get_num_lines() const { return (int)in_.size(); }

    void load_from_pla(const std::string& filePath);
    void write_to_pla(const std::string& filePath) const;
    void print_function() const;

    // inputs = concatenated variable parts in memory order, e.g. "01" + "10000"
    void add_line(const std::string& inputs, int outState);
    // values[v] = state of variable v (0/1 for binary). Returns the output state of
    // the first matching line, -1 if no line matches.
    int evaluate(const std::vector<int>& values) const;

    // Replace the variable at `position` with the function `other`.
    void input_variables(const mv_pla_function& other, int position);
    void input_variables_in_parallel(const mv_pla_function& other, int position, int numberOfParts);

  private:
    struct line_set {
        std::vector<std::string> in;
        std::vector<int> out;
    };

    std::vector<variable> vars_;
    int out_states_ = 0;
    std::vector<std::string> in_;
    std::vector<int> out_;
    std::vector<int> offset_ = {0}; // offset_[v] = first char of variable v in a line

    void rebuild_layout();
    std::string format_line(size_t i) const;
    std::vector<int> allowed_states(const std::string& line, int v) const;
    std::string whatever_line() const;
    void check_substitution(const mv_pla_function& other, int position) const;
    line_set substitute_slice(const mv_pla_function& other, int position, size_t lo, size_t hi,
                              bool withDontCare) const;
    void apply_substitution(const mv_pla_function& other, int position,
                            std::vector<line_set>& parts);
};