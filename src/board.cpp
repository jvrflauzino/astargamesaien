#include "board.hpp"
#include <fstream>
#include <sstream>

bool Board::load_from_string(const std::string& content) {
    std::istringstream iss(content);
    std::string line;
    std::vector<std::vector<int>> temp_rows;

    int r_spec = 0, c_spec = 0;
    bool has_header = false;

    while (std::getline(iss, line)) {
        // Ignora espacos em branco e linhas de comentario (#)
        size_t first = line.find_first_not_of(" \t\r\n");
        if (first == std::string::npos || line[first] == '#') continue;

        std::istringstream ls(line);
        if (!has_header) {
            int val1, val2;
            std::streampos pos = ls.tellg();
            if ((ls >> val1 >> val2) && !(ls >> val1)) {
                r_spec = val1;
                c_spec = val2;
                has_header = true;
                continue;
            } else {
                ls.clear();
                ls.seekg(pos);
            }
        }

        std::vector<int> row_vals;
        std::string token;
        while (ls >> token) {
            if (token == "." || token == "0") {
                row_vals.push_back(0);
            } else {
                row_vals.push_back(std::stoi(token));
            }
        }
        if (!row_vals.empty()) {
            temp_rows.push_back(row_vals);
        }
    }

    if (temp_rows.empty()) return false;

    rows = has_header && r_spec > 0 ? r_spec : static_cast<int>(temp_rows.size());
    cols = has_header && c_spec > 0 ? c_spec : static_cast<int>(temp_rows[0].size());

    initial_grid.clear();
    initial_grid.reserve(rows * cols);
    for (const auto& row : temp_rows) {
        for (int v : row) {
            initial_grid.push_back(v);
        }
    }

    return true;
}

bool Board::load_from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    std::stringstream buffer;
    buffer << file.rdbuf();
    return load_from_string(buffer.str());
}

bool Board::is_goal(const std::vector<int>& grid) const {
    for (int val : grid) {
        if (val > 0) return false;
    }
    return true;
}

int Board::get_remaining_sum(const std::vector<int>& grid) const {
    int sum = 0;
    for (int val : grid) {
        sum += val;
    }
    return sum;
}

int Board::count_active_apples(const std::vector<int>& grid) const {
    int count = 0;
    for (int val : grid) {
        if (val > 0) count++;
    }
    return count;
}

std::vector<Move> Board::get_valid_moves(const std::vector<int>& grid) const {
    std::vector<Move> valid_moves;
    // Percorre todos os retangulos (r1, c1) ate (r2, c2) que sejam "justos"
    // macas removidas corresponde a um unico retangulo de area minima.
    for (int r1 = 0; r1 < rows; ++r1) {
        for (int r2 = r1; r2 < rows; ++r2) {
            for (int c1 = 0; c1 < cols; ++c1) {
                int sum = 0;
                bool has_top = false;
                bool has_bottom = false;

                for (int c2 = c1; c2 < cols; ++c2) {
                    int col_sum = 0;
                    for (int r = r1; r <= r2; ++r) {
                        int v = grid[r * cols + c2];
                        if (v > 0) {
                            col_sum += v;
                            if (r == r1) has_top = true;
                            if (r == r2) has_bottom = true;
                        }
                    }
                    if (c2 == c1 && col_sum == 0) break;
                    sum += col_sum;
                    if (sum > 10) break;
                    // Soma 10 e as 4 bordas (c1, c2, r1, r2) possuem ao menos uma maca ativa
                    if (sum == 10 && col_sum > 0 && has_top && has_bottom) {
                        valid_moves.push_back({r1, c1, r2, c2});
                    }
                }
            }
        }
    }
    return valid_moves;
}

std::vector<int> Board::apply_move(const std::vector<int>& grid, const Move& m) const {
    std::vector<int> new_grid = grid;
    for (int r = m.r1; r <= m.r2; ++r) {
        for (int c = m.c1; c <= m.c2; ++c) {
            new_grid[r * cols + c] = 0;
        }
    }
    return new_grid;
}

std::string Board::to_state_key(const std::vector<int>& grid) const {
    std::string key;
    key.reserve(grid.size());
    for (int v : grid) {
        key.push_back(v > 0 ? static_cast<char>('0' + v) : '.');
    }
    return key;
}

void Board::print(const std::vector<int>& grid, std::ostream& os) const {
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int v = grid[r * cols + c];
            if (v > 0) os << v << " ";
            else os << ". ";
        }
        os << "\n";
    }
}
