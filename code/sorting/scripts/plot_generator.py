#!/usr/bin/env python3
import os, glob
import pandas as pd
import matplotlib.pyplot as plt

MEAS_DIR = "data/measurements"
PLOT_DIR = "data/plots"
os.makedirs(PLOT_DIR, exist_ok=True)

def load_all():
    files = sorted(glob.glob(os.path.join(MEAS_DIR, "measurements_*.csv")))
    if not files:
        raise SystemExit(f"No se encontraron CSV en {MEAS_DIR}/measurements_*.csv")

    dfs = []
    for f in files:
        try:
            df = pd.read_csv(f)
        except Exception as e:
            print(f"[WARN] No se pudo leer {f}: {e}")
            continue

        expected = {"algoritmo","array_nombre","n","time_ms","mem_est_bytes"}
        if not expected.issubset(df.columns):
            print(f"[WARN] {f} le faltan columnas {expected - set(df.columns)} -> ignorado")
            continue

        df["algoritmo"] = df["algoritmo"].astype(str)
        df["n"] = pd.to_numeric(df["n"], errors="coerce")
        df["time_ms"] = pd.to_numeric(df["time_ms"], errors="coerce")
        df["mem_est_bytes"] = pd.to_numeric(df["mem_est_bytes"], errors="coerce")

        df = df.dropna(subset=["n","time_ms","mem_est_bytes"])
        dfs.append(df)

    if not dfs:
        raise SystemExit("No se pudo cargar ningún CSV válido.")
    all_df = pd.concat(dfs, ignore_index=True)
    all_df["algoritmo"] = all_df["algoritmo"].replace({"sort": "stdsort"})
    all_df["input_type"] = all_df["array_nombre"].astype(str).apply(infer_input_type)
    return all_df

def infer_input_type(name: str) -> str:
    s = name.lower()
    if "ascendente" in s:   return "ascendente"
    if "descendente" in s:  return "descendente"
    if "aleatorio" in s:    return "aleatorio"
    return "desconocido"


def plot_time_per_algo(df):
    # Solo valores positivos para log
    df = df[(df["n"] > 0) & (df["time_ms"] > 0)].copy()

    for algo, g in df.groupby("algoritmo"):
        plt.figure()
        for t, sub in g.groupby("input_type"):
            sub = sub.groupby("n", as_index=False)["time_ms"].median().sort_values("n")
            plt.plot(sub["n"], sub["time_ms"], marker="o", linewidth=1, label=t)
        plt.xscale("log", base=10)
        plt.yscale("log", base=10)
        plt.xlabel("n (potencias de 10)")
        plt.ylabel("time_ms (ms, log)")
        plt.title(f"Tiempo vs n — {algo} (por tipo de input)")
        plt.grid(True, which="both", linestyle="--", alpha=0.4)
        plt.legend(title="input")
        out = os.path.join(PLOT_DIR, f"time_vs_n_{algo}.png")
        plt.savefig(out, dpi=160, bbox_inches="tight")
        plt.close()
        print(f"[OK] {out}")

def plot_memory_comparison(df):
    # Filtramos valores válidos
    df = df[(df["n"] > 0) & (df["mem_est_bytes"] > 0)].copy()

    # Convertimos los bytes a Megabytes para que el gráfico sea más legible
    df["mem_mb"] = df["mem_est_bytes"] / (1024 * 1024)

    plt.figure()
    
    # Agrupamos por algoritmo para graficarlos todos en la misma figura
    for algo, g in df.groupby("algoritmo"):
        # Agrupamos por 'n' tomando el valor máximo de memoria usado para ese tamaño
        sub = g.groupby("n", as_index=False)["mem_mb"].max().sort_values("n")
        plt.plot(sub["n"], sub["mem_mb"], marker="s", linewidth=1.5, label=algo)

    plt.xscale("log", base=10)
    plt.xlabel("n (potencias de 10)")
    plt.ylabel("Memoria máxima (MB)")
    plt.title("Comparación de Uso de Memoria vs n")
    plt.grid(True, which="both", linestyle="--", alpha=0.4)
    plt.legend(title="Algoritmo")
    
    out = os.path.join(PLOT_DIR, "memory_comparison_vs_n.png")
    plt.savefig(out, dpi=160, bbox_inches="tight")
    plt.close()
    print(f"[OK] {out}")

def main():
    df = load_all()
    out_all = os.path.join(MEAS_DIR, "all_measurements.csv")
    df.to_csv(out_all, index=False)
    print(f"[OK] {out_all}")

    # Gráficos de tiempo (los que ya tenías)
    plot_time_per_algo(df)
    
    # Nuevo gráfico de comparación de memoria
    plot_memory_comparison(df)

if __name__ == "__main__":
    main()