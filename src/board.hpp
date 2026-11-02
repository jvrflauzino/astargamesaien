#pragma once

#include <vector>
#include <string>
#include <iostream>

// Representa a selecao de um retangulo no tabuleiro entre (r1, c1) e (r2, c2)
struct Move {
    int r1 = 0;
    int c1 = 0;
    int r2 = 0;
    int c2 = 0;

    // Custo do operador: area do retangulo selecionado (numero de celulas cobertas)
    int area() const {
        return (r2 - r1 + 1) * (c2 - c1 + 1);
    }
};

class Board {
public:
    int rows = 0;
    int cols = 0;
    std::vector<int> initial_grid; // 0 = vazio, 1..9 = valor da maca

    Board() = default;

    bool load_from_file(const std::string& path);
    bool load_from_string(const std::string& content);

    // Retorna true se todas as macas foram removidas (todas celulas = 0)
    bool is_goal(const std::vector<int>& grid) const;

    // Retorna a soma de todas as macas ativas na grade
    int get_remaining_sum(const std::vector<int>& grid) const;

    // Retorna a quantidade de macas ativas na grade (usada como heuristica h(n))
    int count_active_apples(const std::vector<int>& grid) const;

    // Encontra todos os retangulos justos (tight bounding boxes) cujas macas ativas somam 10.
    // Celulas vazias (0) intermediarias contam como 0, conforme regras oficiais.
    std::vector<Move> get_valid_moves(const std::vector<int>& grid) const;

    // Aplica o retangulo, zerando todas as celulas contidas nele
    std::vector<int> apply_move(const std::vector<int>& grid, const Move& m) const;

    // Converte a grade em uma string compacta para uso em tabelas hash (visited)
    std::string to_state_key(const std::vector<int>& grid) const;

    // Imprime a grade no terminal
    void print(const std::vector<int>& grid, std::ostream& os = std::cout) const;
};
