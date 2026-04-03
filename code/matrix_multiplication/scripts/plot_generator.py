import argparse
import re
from pathlib import Path
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker


def normalize_columns(df: pd.DataFrame) -> pd.DataFrame:
    # Renombrar columnas a un set estándar
    cols = {c.lower(): c for c in df.columns}
    mapping = {}
    if 'algoritmo' in cols:
        mapping[cols['algoritmo']] = 'algo'
    if 'matriz_nombre' in cols:
        mapping[cols['matriz_nombre']] = 'case'
    if 'algo' in cols:
        mapping[cols['algo']] = 'algo'
    if 'case' in cols:
        mapping[cols['case']] = 'case'
    if 'n' in cols:
        mapping[cols['n']] = 'n'
    if 'time_ms' in cols:
        mapping[cols['time_ms']] = 'time_ms'
    if 'mem_est_bytes' in cols:
        mapping[cols['mem_est_bytes']] = 'mem_est_bytes'
    if 'timestamp_ms' in cols:
        mapping[cols['timestamp_ms']] = 'timestamp_ms'

    df = df.rename(columns=mapping)
    # Tipos
    for col in ['n', 'time_ms', 'mem_est_bytes', 'timestamp_ms']:
        if col in df.columns:
            df[col] = pd.to_numeric(df[col], errors='coerce')
    if 'algo' in df.columns:
        df['algo'] = df['algo'].astype(str)
    if 'case' in df.columns:
        df['case'] = df['case'].astype(str)
    return df


def load_all_measurements(meas_dir: Path) -> pd.DataFrame:
    files = sorted(meas_dir.glob("*.csv"))
    if not files:
        raise FileNotFoundError(f"No se encontraron CSV en {meas_dir}")
    frames = []
    for f in files:
        try:
            df = pd.read_csv(f)
            df['__file'] = f.name
            frames.append(normalize_columns(df))
        except Exception as e:
            print(f"[WARN] No pude leer {f}: {e}")
    if not frames:
        raise RuntimeError("No fue posible cargar ningún CSV válido.")
    df = pd.concat(frames, ignore_index=True)
    # Filtrar registros válidos
    needed = {'algo', 'case', 'n', 'time_ms', 'mem_est_bytes'}
    missing = needed - set(df.columns)
    if missing:
        raise ValueError(f"Faltan columnas requeridas: {missing}")
    df = df.dropna(subset=['n', 'time_ms'])
    df = df[df['n'] > 0]
    return df


def derive_case_group(case_name: str) -> str:
    # Heurística: quitar el prefijo numérico y el sufijo _1.txt/_2.txt
    name = Path(case_name).name
    name = re.sub(r'^\d+[_-]*', '', name)          # remove leading N and sep
    name = re.sub(r'(_[12])?\.txt$', '', name)     # strip _1.txt / _2.txt / .txt
    return name or case_name


def ensure_dir(p: Path):
    p.mkdir(parents=True, exist_ok=True)


def apply_business_filters(df: pd.DataFrame) -> pd.DataFrame:
    """Aplica reglas del análisis:
       - Limitar STRASSEN a N <= 256
    """
    df2 = df.copy()
    mask_strassen = df2['algo'].str.lower() == 'strassen'
    df2 = df2[~mask_strassen | (df2['n'] <= 256)]
    return df2


def plot_t_vs_n_all(df: pd.DataFrame, out_dir: Path):
    # Opcional: quedarte con N ≤ 256 para que coincida con los ticks pedidos
    df_plot = df[(df['n'] <= 256) & (df['time_ms'] > 0)].copy()
    if df_plot.empty:
        print("[WARN] No hay datos válidos (n<=256 y time_ms>0) para 't_vs_n_all'.")
        return

    fig, ax = plt.subplots()
    ax.set_title("Tiempo vs N (todas las mediciones)")
    ax.set_xlabel("N")
    ax.set_ylabel("Tiempo [ms]")

    ax.scatter(df_plot['n'], df_plot['time_ms'], alpha=0.8)

    # X: ticks fijos
    ax.set_xticks([16, 64, 256])

    # Y: escala log10 con etiquetas tipo 10^k
    ax.set_yscale('log')

    y_min = df_plot['time_ms'].min()
    y_max = df_plot['time_ms'].max()
    lo, hi = _pow10_bounds(y_min, y_max)
    ax.set_ylim(lo, hi)

    ax.yaxis.set_major_locator(mticker.LogLocator(base=10.0))
    ax.yaxis.set_minor_locator(mticker.LogLocator(base=10.0, subs=np.arange(2,10)*0.1))
    ax.yaxis.set_major_formatter(mticker.LogFormatterMathtext(base=10.0))
    ax.yaxis.set_minor_formatter(mticker.NullFormatter())

    ax.tick_params(axis='y', which='major', length=6)
    ax.tick_params(axis='y', which='minor', length=3)

    fig.tight_layout()
    fig.savefig(out_dir / "t_vs_n_all.png", dpi=150)
    plt.close(fig)



def _pow10_bounds(ymin, ymax):
    ymin = float(ymin); ymax = float(ymax)
    # evitar problemas si hay valores muy pequeños
    ymin = max(ymin, 1e-12)
    lo = 10.0 ** np.floor(np.log10(ymin))
    hi = 10.0 ** np.ceil(np.log10(ymax))
    if lo == hi:
        lo /= 10.0
        hi *= 10.0
    return lo, hi

def plot_t_vs_n_by_algo(df: pd.DataFrame, out_dir: Path):
    # Limitar a N <= 256 y filtrar tiempos no positivos (log no admite <= 0)
    df_small = df[(df['n'] <= 256) & (df['time_ms'] > 0)].copy()
    if df_small.empty:
        print("[WARN] No hay datos válidos (n<=256 y time_ms>0) para 't_vs_n_by_algo'.")
        return

    g = df_small.groupby(['algo', 'n'], as_index=False)['time_ms'].median()

    fig, ax = plt.subplots()
    ax.set_title("Tiempo vs N por algoritmo (mediana, N ≤ 256)")
    ax.set_xlabel("N")
    ax.set_ylabel("Tiempo [ms]")

    # Curvas por algoritmo
    for algo, sub in g.groupby('algo'):
        sub = sub.sort_values('n')
        ax.plot(sub['n'], sub['time_ms'], marker='o', label=algo)

    # X: ticks fijos
    ax.set_xticks([16, 64, 256])

    # Y: escala log base 10
    ax.set_yscale('log')

    # Límites Y: potencias de 10 que cubran los datos
    y_min = g['time_ms'].min()
    y_max = g['time_ms'].max()
    lo, hi = _pow10_bounds(y_min, y_max)
    ax.set_ylim(lo, hi)

    # Ticks mayores SOLO en potencias de 10, menores típicos entre medias
    ax.yaxis.set_major_locator(mticker.LogLocator(base=10.0))
    ax.yaxis.set_minor_locator(mticker.LogLocator(base=10.0, subs=np.arange(2,10)*0.1))

    # Etiquetas: mayores en formato 10^k, menores sin etiqueta
    ax.yaxis.set_major_formatter(mticker.LogFormatterMathtext(base=10.0))
    ax.yaxis.set_minor_formatter(mticker.NullFormatter())

    # (opcional) para asegurarnos de que las etiquetas se dibujen
    ax.tick_params(axis='y', which='major', length=6)
    ax.tick_params(axis='y', which='minor', length=3)

    ax.legend()
    fig.tight_layout()
    fig.savefig(out_dir / "t_vs_n_by_algo.png", dpi=150)
    plt.close(fig)



def plot_t_vs_n_by_input(df: pd.DataFrame, out_dir: Path):
    # case_group: etiqueta de input (p.ej. "densa_D10_a")
    df = df.copy()
    df['case_group'] = df['case'].map(derive_case_group)

    for algo, sub_algo in df.groupby('algo'):
        # Mediana por N y case_group
        g = sub_algo.groupby(['case_group', 'n'], as_index=False)['time_ms'].median()
        fig, ax = plt.subplots()
        ax.set_title(f"Tiempo vs N por input ({algo})")
        ax.set_xlabel("N")
        ax.set_ylabel("Tiempo [ms]")
        for grp, sub in g.groupby('case_group'):
            sub = sub.sort_values('n')
            ax.plot(sub['n'], sub['time_ms'], marker='o', label=grp)
        ax.legend(fontsize='small')
        fig.tight_layout()
        fname = f"t_vs_n_by_input_{algo}.png"
        fig.savefig(out_dir / fname, dpi=150)
        plt.close(fig)


def plot_mem_vs_n_all(df: pd.DataFrame, out_dir: Path):
    fig, ax = plt.subplots()
    ax.set_title("Memoria estimada vs N (todas las mediciones)")
    ax.set_xlabel("N")
    ax.set_ylabel("Memoria estimada [bytes]")
    ax.scatter(df['n'], df['mem_est_bytes'], alpha=0.8)
    fig.tight_layout()
    fig.savefig(out_dir / "mem_vs_n_all.png", dpi=150)
    plt.close(fig)


def plot_mem_vs_n_by_algo(df: pd.DataFrame, out_dir: Path):
    # Limitar a N ≤ 256
    df_small = df[df['n'] <= 256].copy()
    if df_small.empty:
        print("[WARN] No hay datos válidos (n<=256) para 'mem_vs_n_by_algo'.")
        return

    # Agrupar por algoritmo y N (mediana de memoria estimada)
    g = df_small.groupby(['algo', 'n'], as_index=False)['mem_est_bytes'].median()

    fig, ax = plt.subplots()
    ax.set_title("Memoria estimada vs N por algoritmo (mediana, N ≤ 256)")
    ax.set_xlabel("N")
    ax.set_ylabel("Memoria estimada [bytes]")

    for algo, sub in g.groupby('algo'):
        sub = sub.sort_values('n')
        ax.plot(sub['n'], sub['mem_est_bytes'], marker='o', label=algo)

    # ticks fijos en X
    ax.set_xticks([16, 64, 256])

    # Escala log en Y
    ax.set_yscale('log')

    y_min = g['mem_est_bytes'].min()
    y_max = g['mem_est_bytes'].max()
    lo, hi = _pow10_bounds(y_min, y_max)  # <-- ya tienes esta función definida
    ax.set_ylim(lo, hi)

    ax.yaxis.set_major_locator(mticker.LogLocator(base=10.0))
    ax.yaxis.set_minor_locator(mticker.LogLocator(base=10.0, subs=np.arange(2,10)*0.1))
    ax.yaxis.set_major_formatter(mticker.LogFormatterMathtext(base=10.0))
    ax.yaxis.set_minor_formatter(mticker.NullFormatter())

    ax.tick_params(axis='y', which='major', length=6)
    ax.tick_params(axis='y', which='minor', length=3)

    ax.legend()
    fig.tight_layout()
    fig.savefig(out_dir / "mem_vs_n_by_algo.png", dpi=150)
    plt.close(fig)



def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--measurements-dir", default="data/measurements",
                        help="Directorio con CSVs de mediciones")
    parser.add_argument("--output-dir", default="data/plots",
                        help="Directorio donde guardar los PNGs")
    args = parser.parse_args()

    meas_dir = Path(args.measurements_dir)
    out_dir = Path(args.output_dir)
    ensure_dir(out_dir)

    df = load_all_measurements(meas_dir)
    df = apply_business_filters(df)  # <-- limitar Strassen a N <= 256

    # Generar gráficos
    plot_t_vs_n_all(df, out_dir)
    plot_t_vs_n_by_algo(df, out_dir)
    plot_t_vs_n_by_input(df, out_dir)
    plot_mem_vs_n_all(df, out_dir)
    plot_mem_vs_n_by_algo(df, out_dir)

    print(f"[OK] Gráficos generados en: {out_dir.resolve()}")
    print(" - t_vs_n_all.png")
    print(" - t_vs_n_by_algo.png")
    print(" - t_vs_n_by_input_<algo>.png (uno por algoritmo)")
    print(" - mem_vs_n_all.png")
    print(" - mem_vs_n_by_algo.png")


if __name__ == "__main__":
    main()