import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

# ── Chargement ────────────────────────────────────────────────────────────────
df = pd.read_csv("output_file/genetic_alg/gen_stats_chr_xgen.txt")
generations = sorted(g for g in df["generation"].unique() if g != 0)

# ── Meilleur individu par génération ─────────────────────────────────────────
best = df[df["generation"].isin(generations)].groupby("generation").apply(
    lambda g: g.loc[g["fitness"].idxmax()]
).reset_index(drop=True)

# ── Figure ────────────────────────────────────────────────────────────────────
fig, axes = plt.subplots(4, 1, figsize=(12, 14), sharex=True)

def plot_best(ax, col, ylabel, title, color):
    ax.plot(best["generation"], best[col], color=color, linewidth=2, marker="o", markersize=4)
    if len(generations) > 2:
        z = np.polyfit(best["generation"], best[col], 1)
        p = np.poly1d(z)
        ax.plot(best["generation"], p(best["generation"]),
                color=color, linewidth=1, linestyle="--", alpha=0.5,
                label=f"Tendance ({z[0]:+.4f}/gen)")
        ax.legend(fontsize=9)
    ax.set_ylabel(ylabel, fontsize=11)
    ax.set_title(title, fontsize=12)
    ax.grid(axis="y", linestyle="--", alpha=0.4)

plot_best(axes[0], "fitness",  "Fitness",             "Meilleure fitness par génération",              "seagreen")
plot_best(axes[1], "altitude", "Altitude (m)",        "Altitude du meilleur par génération",           "steelblue")
plot_best(axes[2], "temps",    "Temps récup. (s)",    "Temps de récupération du meilleur par génération", "darkorange")
plot_best(axes[3], "taille",   "Nb situations",       "Taille du chromosome du meilleur par génération",  "mediumpurple")

axes[3].set_xlabel("Génération", fontsize=11)
for ax in axes:
    ax.set_xticks(generations[::max(1, len(generations)//20)])

plt.tight_layout()
plt.savefig("output_file/genetic_alg/gen_best_chr_xgen.png", dpi=150)
plt.show()
print("Figure sauvegardée : output_file/genetic_alg/gen_best_chr_xgen.png")