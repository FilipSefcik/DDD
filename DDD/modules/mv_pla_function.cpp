#include "mv_pla_function.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <thread>
#include <utility>

namespace {
std::vector<std::string> split(const std::string& s, char sep) {
    std::vector<std::string> parts(1);
    for (char c : s) {
        if (c == sep) {
            parts.emplace_back();
        } else {
            parts.back() += c;
        }
    }
    return parts;
}
} // namespace

mv_pla_function::mv_pla_function(std::vector<variable> vars, int outStates)
    : vars_(std::move(vars)), out_states_(outStates) {
    rebuild_layout();
}

void mv_pla_function::rebuild_layout() {
    offset_.assign(1, 0);
    for (const variable& v : vars_) {
        offset_.push_back(offset_.back() + (v.binary ? 1 : v.states));
    }
}

void mv_pla_function::add_line(const std::string& inputs, int outState) {
    if ((int)inputs.size() != offset_.back() || outState < 0 || outState >= out_states_) {
        throw std::runtime_error("add_line: line does not match the variable layout");
    }
    in_.push_back(inputs);
    out_.push_back(outState);
}

// ---------------------------------------------------------------- file I/O

void mv_pla_function::load_from_pla(const std::string& filePath) {
    std::ifstream file(filePath);
    if (! file.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath +
                                 "\nPlease, create the merged directory in a root directory.");
    }

    mv_pla_function result;
    bool haveHeader = false;
    int binaryCount = 0;
    int lineNo = 0;
    auto fail = [&](const std::string& msg) {
        throw std::runtime_error(filePath + ":" + std::to_string(lineNo) + ": " + msg);
    };

    std::string raw;
    while (std::getline(file, raw)) {
        ++lineNo;
        size_t first = raw.find_first_not_of(" \t\r\n");
        if (first == std::string::npos || raw[first] == '#') {
            continue;
        }

        if (raw[first] == '.') {
            std::istringstream iss(raw);
            std::string token;
            iss >> token;
            if (token == ".mv") {
                int n = 0, b = 0;
                iss >> n >> b;
                if (! iss || n < 2 || b < 0 || b > n - 1) {
                    fail("bad .mv header");
                }
                std::vector<int> states(n - b); // multivalued inputs + the output
                for (int& s : states) {
                    iss >> s;
                    if (! iss || s < 1) {
                        fail("bad .mv header");
                    }
                }
                result.vars_.assign(b, variable{true, 2});
                for (size_t i = 0; i + 1 < states.size(); ++i) {
                    result.vars_.push_back(variable{false, states[i]});
                }
                result.out_states_ = states.back();
                result.rebuild_layout();
                binaryCount = b;
                haveHeader = true;
            } else if (token == ".e" || token == ".end") {
                break;
            } // .p and anything else is ignored
            continue;
        }

        if (! haveHeader) {
            fail("the .mv header must come before the lines");
        }

        std::string line;
        for (char c : raw) {
            if (! std::isspace((unsigned char)c)) {
                line += c;
            }
        }
        std::vector<std::string> parts = split(line, '|');
        size_t mvInputs = result.vars_.size() - binaryCount;
        if (parts.size() != mvInputs + 2) {
            fail("expected " + std::to_string(mvInputs + 2) + " parts separated by '|'");
        }

        std::string inputs = parts[0]; // binary part, may be empty
        if ((int)inputs.size() != binaryCount ||
            inputs.find_first_not_of("01-") != std::string::npos) {
            fail("bad binary part");
        }
        for (size_t k = 0; k < mvInputs; ++k) {
            const std::string& p = parts[k + 1];
            if ((int)p.size() != result.vars_[binaryCount + k].states ||
                p.find_first_not_of("01") != std::string::npos) {
                fail("bad part of variable " + std::to_string(binaryCount + k));
            }
            inputs += p;
        }

        const std::string& o = parts.back();
        if ((int)o.size() != result.out_states_ || o.find_first_not_of("01") != std::string::npos ||
            std::count(o.begin(), o.end(), '1') != 1) {
            fail("output part must be one-hot with " + std::to_string(result.out_states_) +
                 " states");
        }
        result.add_line(inputs, (int)o.find('1'));
    }

    if (! haveHeader) {
        throw std::runtime_error("Could not load relevant data from file: " + filePath + "\n");
    }
    *this = std::move(result);
}

std::string mv_pla_function::format_line(size_t i) const {
    std::string bin, mv;
    for (size_t v = 0; v < vars_.size(); ++v) {
        if (vars_[v].binary) {
            bin += in_[i][offset_[v]];
        } else {
            mv += '|' + in_[i].substr(offset_[v], vars_[v].states);
        }
    }
    std::string o(out_states_, '0');
    o[out_[i]] = '1';
    return bin + mv + '|' + o;
}

void mv_pla_function::write_to_pla(const std::string& filePath) const {
    std::ofstream file(filePath);
    if (! file.is_open()) {
        throw std::runtime_error("Error opening file: " + filePath + "\n");
    }

    int binaryCount =
        (int)std::count_if(vars_.begin(), vars_.end(), [](const variable& v) { return v.binary; });
    file << ".mv " << vars_.size() + 1 << ' ' << binaryCount;
    for (const variable& v : vars_) {
        if (! v.binary) {
            file << ' ' << v.states;
        }
    }
    file << ' ' << out_states_ << "\n.p " << in_.size() << "\n\n";
    for (size_t i = 0; i < in_.size(); ++i) {
        file << format_line(i) << '\n';
    }
    file << "\n.e\n";
}

void mv_pla_function::print_function() const {
    for (size_t i = 0; i < in_.size(); ++i) {
        std::cout << format_line(i) << '\n';
    }
    std::cout << in_.size() << " lines\n";
}

int mv_pla_function::evaluate(const std::vector<int>& values) const {
    for (size_t i = 0; i < in_.size(); ++i) {
        bool match = true;
        for (size_t v = 0; v < vars_.size() && match; ++v) {
            if (vars_[v].binary) {
                char c = in_[i][offset_[v]];
                match = (c == '-' || c - '0' == values[v]);
            } else {
                match = (in_[i][offset_[v] + values[v]] == '1');
            }
        }
        if (match) {
            return out_[i];
        }
    }
    return -1;
}

// ------------------------------------------------------------ substitution

// Set of states of variable v that the line accepts.
std::vector<int> mv_pla_function::allowed_states(const std::string& line, int v) const {
    std::vector<int> s;
    if (vars_[v].binary) {
        char c = line[offset_[v]];
        if (c != '1') {
            s.push_back(0); // '0' or '-'
        }
        if (c != '0') {
            s.push_back(1); // '1' or '-'
        }
    } else {
        for (int k = 0; k < vars_[v].states; ++k) {
            if (line[offset_[v] + k] == '1') {
                s.push_back(k);
            }
        }
    }
    return s;
}

// The "don't care" input: '-' for binary variables, all ones for multivalued ones.
std::string mv_pla_function::whatever_line() const {
    std::string s;
    for (const variable& v : vars_) {
        s += v.binary ? std::string(1, '-') : std::string(v.states, '1');
    }
    return s;
}

void mv_pla_function::check_substitution(const mv_pla_function& other, int position) const {
    if (&other == this) {
        throw std::invalid_argument("input_variables: cannot substitute a function into itself");
    }
    if (position < 0 || position >= (int)vars_.size()) {
        throw std::out_of_range("input_variables: position out of range");
    }
    if (other.out_states_ != vars_[position].states) {
        throw std::runtime_error(
            "input_variables: module has " + std::to_string(other.out_states_) +
            " output states, variable at position " + std::to_string(position) + " has " +
            std::to_string(vars_[position].states));
    }
}

// Lines of the result that come from the son's lines [lo, hi).
// A parent line that accepts every state of the variable (all ones, or '-') yields
// one "don't care" line; only the slice with withDontCare == true emits it, so
// the slices can be computed independently and just concatenated.
mv_pla_function::line_set mv_pla_function::substitute_slice(const mv_pla_function& other,
                                                            int position, size_t lo, size_t hi,
                                                            bool withDontCare) const {
    std::vector<std::vector<size_t>> byOut(other.out_states_); // like sort_by_function()
    for (size_t j = lo; j < hi; ++j) {
        byOut[other.out_[j]].push_back(j);
    }
    const std::string whatever = other.whatever_line();
    const int off = offset_[position];
    const int end = offset_[position + 1];

    line_set result;
    for (size_t i = 0; i < in_.size(); ++i) {
        const std::string& line = in_[i];
        std::vector<int> allowed = allowed_states(line, position);

        if ((int)allowed.size() == vars_[position].states) {
            if (withDontCare) {
                result.in.push_back(line.substr(0, off) + whatever + line.substr(end));
                result.out.push_back(out_[i]);
            }
            continue;
        }
        for (int state : allowed) {
            for (size_t j : byOut[state]) {
                result.in.push_back(line.substr(0, off) + other.in_[j] + line.substr(end));
                result.out.push_back(out_[i]);
            }
        }
    }
    return result;
}

void mv_pla_function::apply_substitution(const mv_pla_function& other, int position,
                                         std::vector<line_set>& parts) {
    std::vector<std::string> in;
    std::vector<int> out;
    for (line_set& p : parts) {
        std::move(p.in.begin(), p.in.end(), std::back_inserter(in));
        out.insert(out.end(), p.out.begin(), p.out.end());
    }

    std::vector<variable> vars(vars_.begin(), vars_.begin() + position);
    vars.insert(vars.end(), other.vars_.begin(), other.vars_.end());
    vars.insert(vars.end(), vars_.begin() + position + 1, vars_.end());

    vars_ = std::move(vars);
    in_ = std::move(in);
    out_ = std::move(out);
    rebuild_layout();
}

void mv_pla_function::input_variables(const mv_pla_function& other, int position) {
    check_substitution(other, position);
    std::vector<line_set> parts;
    parts.push_back(substitute_slice(other, position, 0, other.in_.size(), true));
    apply_substitution(other, position, parts);
}

void mv_pla_function::input_variables_in_parallel(const mv_pla_function& other, int position,
                                                  int numberOfParts) {
    check_substitution(other, position);
    size_t n = other.in_.size();
    size_t parts = std::max<size_t>(1, std::min<size_t>(std::max(numberOfParts, 1), n));

    std::vector<line_set> results(parts);
    std::vector<std::thread> threads;
    threads.reserve(parts);

    size_t base = n / parts, remaining = n % parts, lo = 0;
    for (size_t p = 0; p < parts; ++p) {
        size_t hi = lo + base + (p < remaining ? 1 : 0);
        threads.emplace_back([&, p, lo, hi]() {
            results[p] = this->substitute_slice(other, position, lo, hi, p == 0);
        });
        lo = hi;
    }
    for (std::thread& t : threads) {
        t.join();
    }
    apply_substitution(other, position, results);
}