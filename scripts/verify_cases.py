#!/usr/bin/env python3
import subprocess
import json
import os
import sys

def run_solver(board_file, algo="all", timeout_ms=60000):
    cmd = [
        os.path.abspath("fruitbox_solver.exe"),
        "--file", os.path.abspath(board_file),
        "--algo", algo,
        "--timeout", str(timeout_ms)
    ]
    res = subprocess.run(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    if res.returncode != 0:
        print(f"Error running solver on {board_file}: {res.stderr}")
        return None
    try:
        return json.loads(res.stdout)
    except json.JSONDecodeError as e:
        print(f"Failed JSON for {board_file}: {e}")
        print("Output was:\n", res.stdout)
        return None

def format_grid(grid, r, c, highlight=None):
    lines = []
    for i in range(r):
        row_tokens = []
        for j in range(c):
            val = grid[i * c + j]
            ch = str(val) if val > 0 else "."
            if highlight and highlight["r1"] <= i <= highlight["r2"] and highlight["c1"] <= j <= highlight["c2"]:
                row_tokens.append(f"[{ch}]")
            else:
                row_tokens.append(f" {ch} ")
        lines.append("    " + "".join(row_tokens))
    return "\n".join(lines)

def print_step_by_step(initial_grid, r, c, res):
    print(f"\n Passo a Passo: {res['algorithm']} (Custo Total de Area = {res['solution_cost']}) ---")
    grid = list(initial_grid)
    print("  Estado Inicial:")
    print(format_grid(grid, r, c))
    acc_cost = 0
    for idx, m in enumerate(res["moves"], 1):
        r1, c1, r2, c2 = m["r1"], m["c1"], m["r2"], m["c2"]
        area = (r2 - r1 + 1) * (c2 - c1 + 1)
        acc_cost += area
        picked = []
        empty_crossed = 0
        for i in range(r1, r2 + 1):
            for j in range(c1, c2 + 1):
                val = grid[i * c + j]
                if val > 0:
                    picked.append(val)
                else:
                    empty_crossed += 1
        expr = "+".join(str(x) for x in picked)
        extra = f", {empty_crossed} vazia(s) cruzada(s)" if empty_crossed > 0 else ", compacto"
        print(f"\n  Passo {idx}: Retangulo ({r1},{c1})-({r2},{c2}) | Macas: {expr}=10 | Area={area} (g={acc_cost}{extra})")
        print(format_grid(grid, r, c, highlight=m))
        for i in range(r1, r2 + 1):
            for j in range(c1, c2 + 1):
                grid[i * c + j] = 0
    print("\n  Estado Final:")
    print(format_grid(grid, r, c))

def verify_board_file(board_file, show_steps=True):
    print(f" Verificando caso de teste: {os.path.basename(board_file)}")
    
    with open(board_file, "r", encoding="utf-8") as f:
        lines = [line.strip() for line in f if line.strip() and not line.strip().startswith("#")]

    first_line_parts = lines[0].split()
    if len(first_line_parts) == 2 and all(p.isdigit() for p in first_line_parts):
        r, c = int(first_line_parts[0]), int(first_line_parts[1])
        data_lines = lines[1:]
    else:
        r = len(lines)
        c = len(lines[0].split())
        data_lines = lines

    numbers = []
    for line in data_lines:
        row = [int(tok) if tok not in (".", "0") else 0 for tok in line.split()]
        numbers.extend(row)

    total_sum = sum(numbers)
    apple_count = sum(1 for n in numbers if n > 0)
    print(f"Dimensoes: {r} x {c} ({len(numbers)} celulas) | Macas ativas: {apple_count} | Soma total: {total_sum} ({total_sum // 10} jogadas)")

    if total_sum % 10 != 0:
        print(f"ERRO: A soma total ({total_sum}) NAO e multipla de 10! Impossivel limpar 100%.")
        return False

    data = run_solver(board_file, algo="all", timeout_ms=30000)
    if not data:
        return False

    print("\nResultados dos Algoritmos:")
    print(f"{'Algoritmo':<28} | {'Sucesso':<7} | {'Movs':<5} | {'Custo (g)':<9} | {'Nós Exp.':<9} | {'Pico Mem':<9} | {'Tempo (ms)':<10}")
    print("-" * 92)

    all_ok = True
    for res in data["results"]:
        name = res["algorithm"]
        succ = "SIM" if res["success"] else "NAO"
        length = str(res["solution_length"]) if res["success"] else "-"
        cost = str(res["solution_cost"]) if res["success"] else "-"
        expanded = str(res["nodes_expanded"])
        mem = str(res["peak_memory_nodes"])
        time_ms = f"{res['time_ms']:.3f}"
        print(f"{name:<28} | {succ:<7} | {length:<5} | {cost:<9} | {expanded:<9} | {mem:<9} | {time_ms:<10}")
        if not res["success"]:
            all_ok = False
            print(f"   -> Motivo da falha: {res.get('failure_reason', 'Desconhecido')}")

    if show_steps:
        bfs_res = data["results"][0]
        astar_res = data["results"][2]
        if astar_res["success"]:
            print_step_by_step(numbers, r, c, astar_res)
        if bfs_res["success"] and bfs_res["solution_cost"] != astar_res["solution_cost"]:
            print_step_by_step(numbers, r, c, bfs_res)
    return all_ok

if __name__ == "__main__":
    args = [a for a in sys.argv[1:] if a != "--no-steps"]
    show_steps = "--no-steps" not in sys.argv[1:]
    if args:
        files = args
    else:
        test_dir = os.path.join(os.path.dirname(__file__), "..", "casos_teste")
        files = [
            os.path.join(test_dir, f) for f in [
                "caso1_moderado.txt",
                "caso2_intermediario.txt",
                "caso3_complexo.txt"
            ] if os.path.exists(os.path.join(test_dir, f))
        ]
    for f in files:
        verify_board_file(f, show_steps=show_steps)
