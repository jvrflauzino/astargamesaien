#!/usr/bin/env python3
import json
import os
import subprocess

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
os.environ.setdefault("MPLCONFIGDIR", os.path.join(ROOT, "out", ".mplconfig"))
os.makedirs(os.environ["MPLCONFIGDIR"], exist_ok=True)

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
import numpy as np

SOLVER = os.path.join(ROOT, "fruitbox_solver.exe")
CASES_DIR = os.path.join(ROOT, "casos_teste")
DOCS_DIR = os.path.join(ROOT, "docs")
OUT_DIR = os.path.join(ROOT, "out")

CASES = [
    ("Caso 1\n(Moderado 3x4)", "caso1_moderado.txt"),
    ("Caso 2\n(Intermediário 4x5)", "caso2_intermediario.txt"),
    ("Caso 3\n(Complexo 5x6)", "caso3_complexo.txt"),
]


def run_case(filename, runs=5):
    path = os.path.join(CASES_DIR, filename)
    best_data = None
    times = {"BFS (Busca em Largura)": [], "DFS (Busca em Profundidade)": [], "A* (Busca Informada)": []}
    for _ in range(runs):
        res = subprocess.run(
            [SOLVER, "--file", path, "--algo", "all", "--timeout", "60000"],
            capture_output=True, text=True, check=True
        )
        data = json.loads(res.stdout)
        best_data = data
        for r in data["results"]:
            times[r["algorithm"]].append(r["time_ms"])
    for r in best_data["results"]:
        r["time_ms"] = float(np.median(times[r["algorithm"]]))
    return best_data


def plot_charts(all_results):
    os.makedirs(DOCS_DIR, exist_ok=True)
    labels = [c[0] for c in CASES]
    algos = [
        ("BFS (Busca em Largura)", "BFS (Cega)", "#1f77b4"),
        ("DFS (Busca em Profundidade)", "DFS (Cega)", "#ff7f0e"),
        ("A* (Busca Informada)", "A* (Informada)", "#2ca02c"),
    ]

    x = np.arange(len(labels))
    width = 0.25

    # 1. Grafico de Tempo de Execucao (escala logaritmica para visualizar de 0.01ms a 350ms)
    fig, ax = plt.subplots(figsize=(6.8, 3.0), dpi=220)
    for idx, (key, short_name, color) in enumerate(algos):
        vals = [max(res["by_algo"][key]["time_ms"], 0.005) for res in all_results]
        rects = ax.bar(x + (idx - 1) * width, vals, width, label=short_name, color=color, edgecolor="black", linewidth=0.5)
        for rect, v in zip(rects, vals):
            ax.annotate(
                f"{v:.2f}" if v < 10 else f"{v:.1f}",
                xy=(rect.get_x() + rect.get_width() / 2, rect.get_height()),
                xytext=(0, 2), textcoords="offset points",
                ha="center", va="bottom", fontsize=7.5, fontweight="bold"
            )

    ax.set_yscale("log")
    ax.set_ylabel("Tempo de Execução (ms) [escala log]", fontsize=9)
    ax.set_title("Comparação de Tempo de Execução por Caso de Teste", fontsize=10, fontweight="bold")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, fontsize=8.5)
    ax.set_ylim(0.004, 1500)
    ax.grid(axis="y", linestyle="--", alpha=0.5)
    ax.legend(fontsize=8, loc="upper left")
    fig.tight_layout()
    tempo_path = os.path.join(DOCS_DIR, "grafico_tempo.png")
    fig.savefig(tempo_path, dpi=220)
    plt.close(fig)

    # 2. Grafico de Nos Expandidos e Pico de Memoria (escala log)
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(6.8, 2.9), dpi=220)
    for idx, (key, short_name, color) in enumerate(algos):
        exp_vals = [res["by_algo"][key]["nodes_expanded"] for res in all_results]
        mem_vals = [res["by_algo"][key]["peak_memory_nodes"] for res in all_results]

        r1 = ax1.bar(x + (idx - 1) * width, exp_vals, width, label=short_name, color=color, edgecolor="black", linewidth=0.5)
        for rect, v in zip(r1, exp_vals):
            ax1.annotate(
                f"{v}", xy=(rect.get_x() + rect.get_width() / 2, rect.get_height()),
                xytext=(0, 2), textcoords="offset points", ha="center", va="bottom", fontsize=6.5
            )

        r2 = ax2.bar(x + (idx - 1) * width, mem_vals, width, label=short_name, color=color, edgecolor="black", linewidth=0.5)
        for rect, v in zip(r2, mem_vals):
            ax2.annotate(
                f"{v}", xy=(rect.get_x() + rect.get_width() / 2, rect.get_height()),
                xytext=(0, 2), textcoords="offset points", ha="center", va="bottom", fontsize=6.5
            )

    ax1.set_yscale("log")
    ax1.set_ylabel("Nós Expandidos [escala log]", fontsize=8.5)
    ax1.set_title("Nós Expandidos", fontsize=9.5, fontweight="bold")
    ax1.set_xticks(x)
    ax1.set_xticklabels(["Caso 1\n(3x4)", "Caso 2\n(4x5)", "Caso 3\n(5x6)"], fontsize=8)
    ax1.set_ylim(1, 600000)
    ax1.grid(axis="y", linestyle="--", alpha=0.5)
    ax1.legend(fontsize=7, loc="upper left")

    ax2.set_yscale("log")
    ax2.set_ylabel("Pico de Memória (nós) [escala log]", fontsize=8.5)
    ax2.set_title("Consumo de Memória (Pico de Nós)", fontsize=9.5, fontweight="bold")
    ax2.set_xticks(x)
    ax2.set_xticklabels(["Caso 1\n(3x4)", "Caso 2\n(4x5)", "Caso 3\n(5x6)"], fontsize=8)
    ax2.set_ylim(10, 600000)
    ax2.grid(axis="y", linestyle="--", alpha=0.5)
    ax2.legend(fontsize=7, loc="upper left")

    fig.tight_layout()
    mem_path = os.path.join(DOCS_DIR, "grafico_memoria.png")
    fig.savefig(mem_path, dpi=220)
    plt.close(fig)

def main():
    os.makedirs(OUT_DIR, exist_ok=True)
    os.makedirs(DOCS_DIR, exist_ok=True)

    all_results = []
    for label, fname in CASES:
        data = run_case(fname)
        by_algo = {r["algorithm"]: r for r in data["results"]}
        all_results.append({
            "label": label.replace("\n", " "),
            "file": fname,
            "board": data["board"],
            "by_algo": by_algo,
            "results": data["results"],
        })

    out_json = os.path.join(OUT_DIR, "benchmarks.json")
    with open(out_json, "w", encoding="utf-8") as f:
        json.dump(all_results, f, indent=2, ensure_ascii=False)

    plot_charts(all_results)


if __name__ == "__main__":
    main()
