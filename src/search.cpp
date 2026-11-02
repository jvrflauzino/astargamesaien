#include "search.hpp"
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace {
    struct Node {
        std::vector<int> grid;
        int parent = -1;
        Move move;
        int g = 0; // custo acumulado: soma das áreas dos retângulos selecionados
        int h = 0; // heurística: número de maçãs ativas restantes
        int f = 0; // f(n) = g(n) + h(n)
    };

    // Elemento para a fila de prioridades do A*
    struct PriorityItem {
        int node_idx = 0;
        int f = 0;
        int h = 0;

        bool operator>(const PriorityItem& other) const {
            if (f != other.f) return f > other.f; 
            return h > other.h;                   // Desempate: menor h(n) 
        }
    };

    // Reconstrói o caminho de jogadas a partir do nó objetivo.
    // Se não houve objetivo (timeout ou espaço esgotado), devolve o caminho até o nó
    // gerado com menos maçãs restantes (útil para tabuleiros reais não totalmente limpáveis).
    void reconstruct_solution(const Board& board, const std::vector<Node>& tree, int goal_idx, SearchResult& res) {
        int target = goal_idx;
        if (goal_idx == -1) {
            if (res.failure_reason.empty()) {
                res.failure_reason = "Espaço esgotado sem solução";
            }
            int best_apples = -1;
            for (int i = 0; i < static_cast<int>(tree.size()); ++i) {
                int apples = board.count_active_apples(tree[i].grid);
                if (best_apples == -1 || apples < best_apples) {
                    best_apples = apples;
                    target = i;
                }
            }
            if (target == -1) return;
        }

        res.success = (goal_idx != -1);
        res.apples_remaining = board.count_active_apples(tree[target].grid);
        res.solution_cost = tree[target].g;
        int curr = target;
        while (tree[curr].parent != -1) {
            res.moves.push_back(tree[curr].move);
            curr = tree[curr].parent;
        }
        std::reverse(res.moves.begin(), res.moves.end());
        res.solution_length = static_cast<int>(res.moves.size());
    }
}

std::string SearchResult::to_json() const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"algorithm\": \"" << algorithm << "\",\n";
    ss << "  \"success\": " << (success ? "true" : "false") << ",\n";
    ss << "  \"time_ms\": " << std::fixed << std::setprecision(3) << time_ms << ",\n";
    ss << "  \"nodes_expanded\": " << nodes_expanded << ",\n";
    ss << "  \"nodes_generated\": " << nodes_generated << ",\n";
    ss << "  \"peak_memory_nodes\": " << peak_memory_nodes << ",\n";
    ss << "  \"solution_length\": " << solution_length << ",\n";
    ss << "  \"solution_cost\": " << solution_cost << ",\n";
    ss << "  \"apples_remaining\": " << apples_remaining << ",\n";
    if (!failure_reason.empty()) {
        ss << "  \"failure_reason\": \"" << failure_reason << "\",\n";
    }
    ss << "  \"moves\": [\n";
    for (size_t i = 0; i < moves.size(); ++i) {
        const auto& m = moves[i];
        ss << "    {\"step\": " << (i + 1)
           << ", \"r1\": " << m.r1
           << ", \"c1\": " << m.c1
           << ", \"r2\": " << m.r2
           << ", \"c2\": " << m.c2
           << ", \"area\": " << m.area() << "}";
        if (i + 1 < moves.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}";
    return ss.str();
}

SearchResult Solver::solve_bfs() {
    SearchResult res;
    res.algorithm = "BFS (Busca em Largura)";
    auto start_time = std::chrono::high_resolution_clock::now();

    if (board.is_goal(board.initial_grid)) {
        res.success = true;
        return res;
    }

    std::vector<Node> tree;
    tree.reserve(50000);

    std::queue<int> frontier;
    std::unordered_set<std::string> visited;

    tree.push_back({board.initial_grid, -1, {}, 0, 0, 0});
    frontier.push(0);
    visited.insert(board.to_state_key(board.initial_grid));
    res.nodes_generated = 1;

    int goal_idx = -1;

    while (!frontier.empty()) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start_time).count();
        if (elapsed > timeout_ms) {
            res.failure_reason = "Timeout excedido";
            break;
        }

        uint64_t current_mem = visited.size() + frontier.size();
        if (current_mem > res.peak_memory_nodes) res.peak_memory_nodes = current_mem;

        int curr_idx = frontier.front();
        frontier.pop();
        res.nodes_expanded++;

        std::vector<int> curr_grid = tree[curr_idx].grid;
        int curr_g = tree[curr_idx].g;

        for (const auto& m : board.get_valid_moves(curr_grid)) {
            std::vector<int> next_grid = board.apply_move(curr_grid, m);
            std::string key = board.to_state_key(next_grid);
            int next_g = curr_g + m.area();

            if (board.is_goal(next_grid)) {
                int next_idx = static_cast<int>(tree.size());
                tree.push_back({next_grid, curr_idx, m, next_g, 0, 0});
                res.nodes_generated++;
                goal_idx = next_idx;
                break;
            }

            if (visited.find(key) == visited.end()) {
                visited.insert(key);
                int next_idx = static_cast<int>(tree.size());
                tree.push_back({next_grid, curr_idx, m, next_g, 0, 0});
                frontier.push(next_idx);
                res.nodes_generated++;
            }
        }

        if (goal_idx != -1) break;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    reconstruct_solution(board, tree, goal_idx, res);
    return res;
}

SearchResult Solver::solve_dfs() {
    SearchResult res;
    res.algorithm = "DFS (Busca em Profundidade)";
    auto start_time = std::chrono::high_resolution_clock::now();

    if (board.is_goal(board.initial_grid)) {
        res.success = true;
        return res;
    }

    std::vector<Node> tree;
    tree.reserve(10000);

    std::vector<int> stack;
    std::unordered_set<std::string> visited;

    tree.push_back({board.initial_grid, -1, {}, 0, 0, 0});
    stack.push_back(0);
    visited.insert(board.to_state_key(board.initial_grid));
    res.nodes_generated = 1;

    int goal_idx = -1;

    while (!stack.empty()) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start_time).count();
        if (elapsed > timeout_ms) {
            res.failure_reason = "Timeout excedido";
            break;
        }

        uint64_t current_mem = visited.size() + stack.size();
        if (current_mem > res.peak_memory_nodes) res.peak_memory_nodes = current_mem;

        int curr_idx = stack.back();
        stack.pop_back();
        res.nodes_expanded++;

        std::vector<int> curr_grid = tree[curr_idx].grid;
        int curr_g = tree[curr_idx].g;

        for (const auto& m : board.get_valid_moves(curr_grid)) {
            std::vector<int> next_grid = board.apply_move(curr_grid, m);
            std::string key = board.to_state_key(next_grid);
            int next_g = curr_g + m.area();

            if (board.is_goal(next_grid)) {
                int next_idx = static_cast<int>(tree.size());
                tree.push_back({next_grid, curr_idx, m, next_g, 0, 0});
                res.nodes_generated++;
                goal_idx = next_idx;
                break;
            }

            if (visited.find(key) == visited.end()) {
                visited.insert(key);
                int next_idx = static_cast<int>(tree.size());
                tree.push_back({next_grid, curr_idx, m, next_g, 0, 0});
                stack.push_back(next_idx);
                res.nodes_generated++;
            }
        }

        if (goal_idx != -1) break;
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    reconstruct_solution(board, tree, goal_idx, res);
    return res;
}

SearchResult Solver::solve_astar() {
    SearchResult res;
    res.algorithm = "A* (Busca Informada)";
    auto start_time = std::chrono::high_resolution_clock::now();

    if (board.is_goal(board.initial_grid)) {
        res.success = true;
        return res;
    }

    std::vector<Node> tree;
    tree.reserve(50000);

    std::priority_queue<PriorityItem, std::vector<PriorityItem>, std::greater<PriorityItem>> open_set;
    std::unordered_map<std::string, int> best_g;

    // AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA
    int h0 = board.count_active_apples(board.initial_grid);
    std::string start_key = board.to_state_key(board.initial_grid);

    tree.push_back({board.initial_grid, -1, {}, 0, h0, h0});
    open_set.push({0, h0, h0});
    best_g[start_key] = 0;
    res.nodes_generated = 1;

    int goal_idx = -1;

    while (!open_set.empty()) {
        auto now = std::chrono::high_resolution_clock::now();
        double elapsed = std::chrono::duration<double, std::milli>(now - start_time).count();
        if (elapsed > timeout_ms) {
            res.failure_reason = "Timeout excedido";
            break;
        }

        uint64_t current_mem = best_g.size() + open_set.size();
        if (current_mem > res.peak_memory_nodes) res.peak_memory_nodes = current_mem;

        PriorityItem item = open_set.top();
        open_set.pop();

        int curr_idx = item.node_idx;
        std::vector<int> curr_grid = tree[curr_idx].grid;
        int curr_g = tree[curr_idx].g;
        std::string curr_key = board.to_state_key(curr_grid);

        if (curr_g > best_g[curr_key]) continue;
        res.nodes_expanded++;

        if (board.is_goal(curr_grid)) {
            goal_idx = curr_idx;
            break;
        }

        for (const auto& m : board.get_valid_moves(curr_grid)) {
            std::vector<int> next_grid = board.apply_move(curr_grid, m);
            std::string next_key = board.to_state_key(next_grid);

            int next_g = curr_g + m.area();

            auto it = best_g.find(next_key);
            if (it != best_g.end() && next_g >= it->second) {
                continue;
            }

            int h = board.count_active_apples(next_grid);
            int f = next_g + h;

            best_g[next_key] = next_g;
            int next_idx = static_cast<int>(tree.size());
            tree.push_back({next_grid, curr_idx, m, next_g, h, f});
            open_set.push({next_idx, f, h});
            res.nodes_generated++;
        }
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    res.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
    reconstruct_solution(board, tree, goal_idx, res);
    return res;
}
