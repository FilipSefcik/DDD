#include "derivative_table.hpp"

#include <algorithm>
#include <iomanip>
#include <istream>
#include <limits>
#include <sstream>
#include <stdexcept>

std::vector<Transition> DerivativeTable::make_transitions(int stateCount, bool restricted) {
    std::vector<Transition> result;
    int maxState = stateCount - 1;
    if (restricted) {
        if (maxState >= 1)
            result.push_back({1, 0});
        return result;
    }
    for (int from = maxState; from >= 1; --from)
        for (int to = from - 1; to >= 0; --to)
            result.push_back({from, to});
    return result;
}

void DerivativeTable::init(int sysStateCount, bool sysRestricted, int compStateCount,
                           bool compRestricted) {
    init(make_transitions(sysStateCount, sysRestricted),
         make_transitions(compStateCount, compRestricted));
}

void DerivativeTable::init(std::vector<Transition> r, std::vector<Transition> c) {
    rows = std::move(r);
    cols = std::move(c);
    values.assign(rows.size(), std::vector<double>(cols.size(), 0.0));
}

void DerivativeTable::clear() {
    rows.clear();
    cols.clear();
    values.clear();
}

bool DerivativeTable::has(const Transition& r, const Transition& c) const {
    return std::find(rows.begin(), rows.end(), r) != rows.end() &&
           std::find(cols.begin(), cols.end(), c) != cols.end();
}

double& DerivativeTable::at(const Transition& r, const Transition& c) {
    return values.at(row_index(r)).at(col_index(c));
}

const double& DerivativeTable::at(const Transition& r, const Transition& c) const {
    return values.at(row_index(r)).at(col_index(c));
}

std::string DerivativeTable::to_string() const {
    std::ostringstream out;
    out << std::setprecision(std::numeric_limits<double>::max_digits10);
    out << rows.size() << ' ' << cols.size();
    for (const auto& r : rows)
        out << ' ' << r.first << ' ' << r.second;
    for (const auto& c : cols)
        out << ' ' << c.first << ' ' << c.second;
    for (const auto& row : values)
        for (double v : row)
            out << ' ' << v;
    return out.str();
}

DerivativeTable DerivativeTable::read(std::istream& in) {
    size_t rowCount, colCount;
    if (! (in >> rowCount >> colCount))
        throw std::invalid_argument("DerivativeTable: missing table size");

    const auto read_transitions = [&in](size_t count) {
        std::vector<Transition> out;
        out.reserve(count);
        for (size_t i = 0; i < count; ++i) {
            Transition t;
            if (! (in >> t.first >> t.second))
                throw std::invalid_argument("DerivativeTable: missing transition");
            out.push_back(t);
        }
        return out;
    };

    DerivativeTable table;
    auto r = read_transitions(rowCount);
    auto c = read_transitions(colCount);
    table.init(std::move(r), std::move(c));

    for (size_t i = 0; i < rowCount; ++i)
        for (size_t j = 0; j < colCount; ++j)
            if (! (in >> table.values[i][j]))
                throw std::invalid_argument("DerivativeTable: missing value");
    return table;
}

DerivativeTable DerivativeTable::from_string(const std::string& s) {
    std::istringstream in(s);
    DerivativeTable table = read(in);
    std::string trailing;
    if (in >> trailing)
        throw std::invalid_argument("DerivativeTable: unexpected trailing data");
    return table;
}

size_t DerivativeTable::row_index(const Transition& r) const {
    auto it = std::find(rows.begin(), rows.end(), r);
    if (it == rows.end())
        throw std::out_of_range("DerivativeTable: row pair not found");
    return static_cast<size_t>(it - rows.begin());
}

size_t DerivativeTable::col_index(const Transition& c) const {
    auto it = std::find(cols.begin(), cols.end(), c);
    if (it == cols.end())
        throw std::out_of_range("DerivativeTable: column pair not found");
    return static_cast<size_t>(it - cols.begin());
}

DerivativeTable DerivativeTable::multiply(const DerivativeTable& other) const {
    if (cols.size() != other.rows.size())
        throw std::invalid_argument(
            "DerivativeTable::multiply: component transitions of the first table do not match "
            "system transitions of the second");

    // For each column k of this, find the row of other with the same transition.
    std::vector<size_t> match(cols.size());
    for (size_t k = 0; k < cols.size(); ++k) {
        auto it = std::find(other.rows.begin(), other.rows.end(), cols[k]);
        if (it == other.rows.end())
            throw std::invalid_argument(
                "DerivativeTable::multiply: transition " + std::to_string(cols[k].first) + "->" +
                std::to_string(cols[k].second) + " is missing in the second table");
        match[k] = static_cast<size_t>(it - other.rows.begin());
    }

    DerivativeTable result;
    result.init(rows, other.cols); // zero-filled: system of first x component of second

    for (size_t i = 0; i < rows.size(); ++i)
        for (size_t j = 0; j < other.cols.size(); ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < cols.size(); ++k)
                sum += values[i][k] * other.values[match[k]][j];
            result.values[i][j] = sum;
        }
    return result;
}