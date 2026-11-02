#include "board.hpp"
#include "search.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

void print_usage(const char* prog) {
    std::cerr << "Uso: " << prog << " [opcoes]\n"
              << "Opcoes:\n"
              << "  --file <caminho>     Arquivo de entrada com o tabuleiro\n"
              << "  --algo <bfs|dfs|astar|all> Algoritmo de busca a executar (padrao: all)\n"
              << "  --timeout <ms>       Tempo limite em milissegundos (padrao: 60000)\n"
              << "  --output <caminho>   Arquivo para salvar o resultado JSON (padrao: stdout)\n"
              << "  --help               Exibe esta mensagem de ajuda\n";
}

int main(int argc, char* argv[]) {
    std::string file_path = "";
    std::string algo = "all";
    int timeout_ms = 60000;
    std::string output_path = "";

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--file" && i + 1 < argc) {
            file_path = argv[++i];
        } else if (arg == "--algo" && i + 1 < argc) {
            algo = argv[++i];
        } else if (arg == "--timeout" && i + 1 < argc) {
            timeout_ms = std::stoi(argv[++i]);
        } else if (arg == "--output" && i + 1 < argc) {
            output_path = argv[++i];
        } else if (arg == "--help") {
            print_usage(argv[0]);
            return 0;
        }
    }

    Board board;
    if (!file_path.empty()) {
        if (!board.load_from_file(file_path)) {
            std::cerr << "Erro: Nao foi possivel abrir ou carregar o arquivo: " << file_path << "\n";
            return 1;
        }
    } else {
        std::string content((std::istreambuf_iterator<char>(std::cin)),
                             std::istreambuf_iterator<char>());
        if (!board.load_from_string(content)) {
            std::cerr << "Erro: Nao foi possivel ler tabuleiro da entrada padrao (stdin).\n";
            return 1;
        }
    }

    Solver solver(board, timeout_ms);
    std::vector<SearchResult> results;

    if (algo == "bfs" || algo == "all") {
        results.push_back(solver.solve_bfs());
    }
    if (algo == "dfs" || algo == "all") {
        results.push_back(solver.solve_dfs());
    }
    if (algo == "astar" || algo == "all") {
        results.push_back(solver.solve_astar());
    }

    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"board\": {\n";
    ss << "    \"rows\": " << board.rows << ",\n";
    ss << "    \"cols\": " << board.cols << ",\n";
    ss << "    \"total_cells\": " << (board.rows * board.cols) << ",\n";
    ss << "    \"initial_apple_count\": " << board.count_active_apples(board.initial_grid) << ",\n";
    ss << "    \"initial_sum\": " << board.get_remaining_sum(board.initial_grid) << "\n";
    ss << "  },\n";
    ss << "  \"results\": [\n";
    for (size_t i = 0; i < results.size(); ++i) {
        ss << results[i].to_json();
        if (i + 1 < results.size()) ss << ",";
        ss << "\n";
    }
    ss << "  ]\n";
    ss << "}\n";

    std::string json_str = ss.str();

    if (!output_path.empty()) {
        std::ofstream out(output_path);
        if (out.is_open()) {
            out << json_str;
        } else {
            std::cerr << "Erro ao gravar arquivo de saida: " << output_path << "\n";
        }
    } else {
        std::cout << json_str;
    }

    return 0;
}
