import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.cm as cm

# ── Chargement ────────────────────────────────────────────────────────────────
df = pd.read_csv("output/gen_stats.txt")
generations = sorted(g for g in df["generation"].unique() if g != 0)
n_gen = len(generations)

# ── Calcul des stats par génération ──────────────────────────────────────────
def compute_stats(df, generations, col):
    stats = []
    for g in generations:
        sub = df[df["generation"] == g][col].values
        stats.append({
            "gen"    : g,
            "min"    : np.min(sub),
            "max"    : np.max(sub),
            "median" : np.median(sub),
            "q25"    : np.percentile(sub, 25),
            "q75"    : np.percentile(sub, 75),
            "all"    : sub,
        })
    return stats

stats_temps    = compute_stats(df, generations, "temps")
stats_altitude = compute_stats(df, generations, "altitude")
stats_fitness  = compute_stats(df, generations, "fitness")

# ── Fonction de tracé générique ───────────────────────────────────────────────
def plot_dispersion(ax, stats, ylabel, title):
    cmap = cm.get_cmap("coolwarm", n_gen)

    for i, s in enumerate(stats):
        if i % 2 == 0:
            ax.axvspan(s["gen"] - 0.5, s["gen"] + 0.5, color="#f5f5f5", zorder=0)

    for s in stats:
        g = s["gen"]
        jitter = np.random.uniform(-0.15, 0.15, size=len(s["all"]))
        ax.scatter(g + jitter, s["all"],
                   color=cmap(g / max(n_gen - 1, 1)),
                   alpha=0.35, s=18, zorder=2)

    for s in stats:
        g = s["gen"]
        ax.plot([g, g], [s["q25"], s["q75"]],
                color="steelblue", linewidth=3, alpha=0.6, zorder=3)

    for s in stats:
        g = s["gen"]
        ax.scatter(g, s["min"], color="royalblue", s=90, zorder=5,
                   marker="v", label="Min" if s == stats[0] else "")
        ax.scatter(g, s["max"], color="firebrick", s=90, zorder=5,
                   marker="^", label="Max" if s == stats[0] else "")

    medians = [s["median"] for s in stats]
    ax.plot(generations, medians,
            color="darkorange", linewidth=2, marker="o", markersize=7,
            zorder=6, label="Médiane")

    if n_gen > 2:
        z = np.polyfit(generations, medians, 1)
        p = np.poly1d(z)
        ax.plot(generations, p(generations),
                color="darkorange", linewidth=1, linestyle="--", alpha=0.5,
                label=f"Tendance médiane ({z[0]:+.2f}/gen)")

    ax.set_xlabel("Génération", fontsize=11)
    ax.set_ylabel(ylabel, fontsize=11)
    ax.set_title(title, fontsize=12)
    ax.set_xticks(generations)
    ax.legend(loc="upper right", fontsize=9)
    ax.grid(axis="y", linestyle="--", alpha=0.4)


def plot_fitness(ax, stats):
    """Tracé spécifique fitness : meilleur individu + médiane + pire."""
    cmap = cm.get_cmap("coolwarm", n_gen)

    for i, s in enumerate(stats):
        if i % 2 == 0:
            ax.axvspan(s["gen"] - 0.5, s["gen"] + 0.5, color="#f5f5f5", zorder=0)

    # Points individuels
    for s in stats:
        g = s["gen"]
        jitter = np.random.uniform(-0.15, 0.15, size=len(s["all"]))
        ax.scatter(g + jitter, s["all"],
                   color=cmap(g / max(n_gen - 1, 1)),
                   alpha=0.35, s=18, zorder=2)

    # Meilleur (max fitness) en vert, pire (min) en rouge
    best    = [s["max"]    for s in stats]
    worst   = [s["min"]    for s in stats]
    medians = [s["median"] for s in stats]

    ax.plot(generations, best,
            color="seagreen", linewidth=2, marker="^", markersize=8,
            zorder=6, label="Meilleur")
    ax.plot(generations, worst,
            color="crimson", linewidth=2, marker="v", markersize=8,
            zorder=6, label="Pire")
    ax.plot(generations, medians,
            color="darkorange", linewidth=2, marker="o", markersize=7,
            zorder=6, label="Médiane")

    # Zone entre meilleur et pire
    ax.fill_between(generations, worst, best,
                    color="steelblue", alpha=0.08, zorder=1, label="Étendue")

    # Ligne de tendance sur le meilleur
    if n_gen > 2:
        z = np.polyfit(generations, best, 1)
        p = np.poly1d(z)
        ax.plot(generations, p(generations),
                color="seagreen", linewidth=1, linestyle="--", alpha=0.5,
                label=f"Tendance meilleur ({z[0]:+.2f}/gen)")

    # Ligne zéro si elle est dans la plage (sépare crash / sauvetage)
    y_min, y_max = ax.get_ylim()
    if y_min < 0 < y_max:
        ax.axhline(0, color="black", linewidth=1, linestyle=":", alpha=0.5,
                   label="Seuil crash / sauvetage")

    ax.set_xlabel("Génération", fontsize=11)
    ax.set_ylabel("Fitness", fontsize=11)
    ax.set_title("Évolution de la fitness par génération\n"
                 "▲ Meilleur  ▼ Pire  ● Médiane  | zone = étendue totale",
                 fontsize=12)
    ax.set_xticks(generations)
    ax.legend(loc="upper left", fontsize=9)
    ax.grid(axis="y", linestyle="--", alpha=0.4)


# ── Figure : 3 subplots ───────────────────────────────────────────────────────
fig, (ax1, ax2, ax3) = plt.subplots(3, 1, figsize=(12, 14), sharex=True)

plot_dispersion(ax1, stats_temps,
                ylabel="Temps de récupération (s)",
                title="Dispersion du temps de récupération par génération\n"
                      "▲ Max  ▼ Min  ● Médiane  | barre = IQR Q25–Q75")

plot_dispersion(ax2, stats_altitude,
                ylabel="Altitude de récupération (m)",
                title="Dispersion de l'altitude de récupération par génération")

plot_fitness(ax3, stats_fitness)

plt.tight_layout()
plt.savefig("output/gen_dispersion.png", dpi=150)
plt.show()
print("Figure sauvegardée : output/gen_dispersion.png")