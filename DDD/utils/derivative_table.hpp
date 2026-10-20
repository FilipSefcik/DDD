#pragma once
#include <iosfwd>
#include <string>
#include <utility>
#include <vector>

using Transition = std::pair<int, int>; // (from, to), from > to

struct DerivativeTable {
    std::vector<Transition> rows; // system transitions (structure 1)
    std::vector<Transition> cols; // component transitions (structure 2)
    std::vector<std::vector<double>> values; // values[r][c]

    // ---------- building ----------

    // All descending transitions for states 0..stateCount-1.
    // restricted == true -> only 1->0 (if the structure has at least 2 states).
    static std::vector<Transition> make_transitions(int stateCount, bool restricted);

    // Prepares an empty (zero-filled) table from state counts and restriction flags.
    void init(int sysStateCount, bool sysRestricted, int compStateCount, bool compRestricted);

    // Prepares an empty table from explicit pair lists.
    void init(std::vector<Transition> r, std::vector<Transition> c);

    void clear();

    // ---------- queries ----------

    // True if there are no row pairs or no column pairs, so no value can exist.
    bool is_empty() const { return rows.empty() || cols.empty(); }

    bool has(const Transition& r, const Transition& c) const;

    // Lookup by the actual pair, e.g. at({1,0}, {2,0}); throws std::out_of_range if missing.
    double& at(const Transition& r, const Transition& c);
    const double& at(const Transition& r, const Transition& c) const;

    // ---------- serialization ----------
    // Format: <rowCount> <colCount> <r.from r.to>... <c.from c.to>... <values row by row>

    std::string to_string() const;

    // Reads one table from a stream and leaves the stream after it.
    // Throws std::invalid_argument on bad input.
    static DerivativeTable read(std::istream& in);

    // Parses a whole string; trailing data is an error.
    static DerivativeTable from_string(const std::string& s);

    // ---------- operations ----------

    // Chain multiplication: this (M1 x M2) * other (M2 x X0) -> (M1 x X0).
    // The columns of this must be the same set of transitions as the rows of other
    // (same variable: component in the first table, system in the second).
    // Throws std::invalid_argument if they don't match.
    DerivativeTable multiply(const DerivativeTable& other) const;

  private:
    size_t row_index(const Transition& r) const;
    size_t col_index(const Transition& c) const;
};