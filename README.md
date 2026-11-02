# estrutura do projeto:

- src/
    board.hpp, board.cpp   : Modelagem do tabuleiro, operadores e custos.
    search.hpp, search.cpp : Implementacao da Busca Cega (BFS/DFS) e Informada (A*).
    main.cpp               : Interface de linha de comando (CLI) do solucionador.
- casos_teste/
    caso1_moderado.txt     : Caso de teste moderado (grade 3x4).
    caso2_intermediario.txt: Caso de teste intermediario (grade 4x5).  #aq pode criar arquivo txt seguindo o modelo pra simular novas solucoes
    caso3_complexo.txt     : Caso de teste complexo (grade 5x6).
- scripts/
    verify_cases.py        : Executa os 3 casos e exibe a solucao passo a passo.
    run_benchmarks.py      : Executa a bateria experimental e gera os graficos.
- docs/
    relatorio.pdf          : Relatorio final formatado conforme as normas da disciplina. 
- Makefile                 : Script de compilacao C++17.

tex file com o relatório, mas ja tem o relatório compilado em formato pdf.


# Para rodar o projeto:
- Compilador C++ com suporte a C++17 (g++ / MinGW ou equivalente).
- Python 3 (serve para exibir o passo a passo visual formatado no
  terminal via scripts/verify_cases.py).

Para compilar o motor de busca em C++:

   usando Make:
    mingw32-make pro windows ou make no Linux

  pra compilar direto rodar o comando:
    g++ -std=c++17 -O3 -Wall -Wextra -Isrc src/main.cpp src/board.cpp src/search.cpp -o fruitbox_solver.exe



# Exibicao visual passo a passo:
Executa os 3 casos de teste nos algoritmos de busca, mostra a tabela comparativa
de tempo/memoria/custo e imprime o estado do tabuleiro a cada jogada:

    python scripts/verify_cases.py

Para executar apenas um caso especifico:

    python scripts/verify_cases.py casos_teste/caso1_moderado.txt
    python scripts/verify_cases.py casos_teste/caso2_intermediario.txt
    etc..

Execucao direta do C++ (saida em JSON q so serve pra dps fazer o parse e transformar em algo visual com o python ent isso aq eh so pra rodar o motor, o de cima ja puxa esses códigos diretamente):

    ./fruitbox_solver.exe --file casos_teste/caso1_moderado.txt --algo all
    ./fruitbox_solver.exe --file casos_teste/caso2_intermediario.txt --algo astar
    ./fruitbox_solver.exe --file casos_teste/caso3_complexo.txt --algo bfs

Parametros aceitos pelo binario:
    --file <caminho>            Arquivo com a grade de entrada.
    --algo <bfs|dfs|astar|all>  Estrategia de busca a executar (padrao: all).
    --timeout <ms>              Tempo limite em milissegundos (padrao: 60000).
