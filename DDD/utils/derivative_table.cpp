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