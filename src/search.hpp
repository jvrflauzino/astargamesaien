#pragma once

#include "board.hpp"
#include <cstdint>
#include <vector>
#include <string>

struct SearchResult {
    std::string algorithm;
    bool success = false;
    double time_ms = 0.0;
    uint64_t nodes_expanded = 0;
    uint64_t nodes_generated = 0;
    uint64_t peak_memory_nodes = 0;
    int solution_length = 0;  // numero de jogadas na solucao
    int solution_cost = 0;    // custo total g(n): soma das areas dos retangulos selecionados
    int apples_remaining = 0; // 0 quando success caso contrario, macas restantes no melhor no encontrado
    std::vector<Move> moves;  // solucao completa, ou caminho ate o melhor no parcial (timeout/sem solucao)
    std::string failure_reason = "";

    std::string to_json() const;
};

class Solver {
public:
    const Board& board;
    int timeout_ms = 60000;

    explicit Solver(const Board& b, int timeout_limit_ms = 60000)
        : board(b), timeout_ms(timeout_limit_ms) {}

    // Busca Cega: Largura (BFS)
    SearchResult solve_bfs();

    // Busca Cega: Profundidade (DFS)
    SearchResult solve_dfs();

    // Busca Informada: A* com g(n) = soma das areas dos retangulos e heuristica de macas restante
    SearchResult solve_astar();
};
