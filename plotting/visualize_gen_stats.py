from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd


def main() -> None:
    root = Path(__file__).resolve().parent.parent
    csv_path = root / "output_file" / "genetic_alg" / "gen_stats_chr_1000gen.txt"
    out_path = root / "output_file" / "genetic_alg" / "gen_best_chr_xgen.png"

    if not csv_path.exists():
        print(f"Fichier introuvable: {csv_path}")
        return

    df = pd.read_csv(csv_path)
    required_cols = {"generation", "fitness", "altitude", "temps", "taille"}
    missing_cols = required_cols - set(df.columns)
    if missing_cols:
        print(f"Colonnes manquantes dans {csv_path.name}: {sorted(missing_cols)}")
        return

    if df.empty:
        print(f"Aucune donnée dans {csv_path}")
        return

    df = df.copy()
    for col in ["generation", "fitness", "altitude", "temps", "taille"]:
        df[col] = pd.to_numeric(df[col], errors="coerce")
    df = df.dropna(subset=["generation", "fitness", "altitude", "temps", "taille"])

    if df.empty:
        print(f"Aucune ligne numérique exploitable dans {csv_path}")
        return

    df["generation"] = df["generation"].astype(int)
    generations = sorted(g for g in df["generation"].unique() if g != 0)
    if not generations:
        generations = sorted(df["generation"].unique())

    if not generations:
        print(f"Aucune génération disponible dans {csv_path}")
        return

    df_gen = df[df["generation"].isin(generations)]
    if df_gen.empty:
        print(f"Aucune donnée pour les générations sélectionnées dans {csv_path}")
        return

    idx_best = df_gen.groupby("generation")["fitness"].idxmax()
    best = df_gen.loc[idx_best].sort_values("generation").reset_index(drop=True)

    fig, axes = plt.subplots(4, 1, figsize=(12, 14), sharex=True)

    def plot_best(ax, col, ylabel, title, color):
        ax.plot(best["generation"], best[col], color=color, linewidth=2, marker="o", markersize=4)
        if len(generations) > 2:
            z = np.polyfit(best["generation"], best[col], 1)
            p = np.poly1d(z)
            ax.plot(
                best["generation"],
                p(best["generation"]),
                color=color,
                linewidth=1,
                linestyle="--",
                alpha=0.5,
                label=f"Tendance ({z[0]:+.4f}/gen)",
            )
            ax.legend(fontsize=9)
        ax.set_ylabel(ylabel, fontsize=11)
        ax.set_title(title, fontsize=12)
        ax.grid(axis="y", linestyle="--", alpha=0.4)

    plot_best(axes[0], "fitness", "Fitness", "Meilleure fitness par génération", "seagreen")
    plot_best(axes[1], "altitude", "Altitude (m)", "Altitude du meilleur par génération", "steelblue")
    plot_best(
        axes[2],
        "temps",
        "Temps récup. (s)",
        "Temps de récupération du meilleur par génération",
        "darkorange",
    )
    plot_best(
        axes[3],
        "taille",
        "Nb situations",
        "Taille du chromosome du meilleur par génération",
        "mediumpurple",
    )

    axes[3].set_xlabel("Génération", fontsize=11)
    for ax in axes:
        ax.set_xticks(generations[:: max(1, len(generations) // 20)])

    plt.tight_layout()
    plt.savefig(out_path, dpi=150)
    plt.show()
    print(f"Figure sauvegardée : {out_path}")


if __name__ == "__main__":
    main()