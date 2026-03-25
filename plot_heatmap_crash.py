from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import matplotlib.cm as cm
import matplotlib.colors as mcolors
from matplotlib.patches import Patch


def find_batch_file(root: Path) -> Path:
    output_dir = root / "output_file"
    batch_file = output_dir / "batch_results.csv"
    if not batch_file.exists():
        raise SystemExit(
            "Fichier introuvable: output_file/batch_results.csv. "
            "Lance d'abord RUN_BATCH pour le générer."
        )
    return batch_file


def read_duree_batch(root: Path) -> str:
    config_path = root / "Config.txt"
    if config_path.exists():
        for line in config_path.read_text(encoding="utf-8").splitlines():
            line = line.strip()
            if line.startswith("duree_batch"):
                parts = line.split("=", 1)
                if len(parts) == 2:
                    return parts[1].strip()
    return "?"


def main() -> None:
    root = Path(__file__).resolve().parent
    output_plot_dir = root / "output_plot"
    output_plot_dir.mkdir(parents=True, exist_ok=True)
    csv_path = find_batch_file(root)
    duree_batch = read_duree_batch(root)
    df = pd.read_csv(csv_path)

    required = {"cmd_profondeur", "cmd_thrust", "crash", "final_altitude"}
    missing = required - set(df.columns)
    if missing:
        raise SystemExit(f"Colonnes manquantes dans {csv_path.name}: {sorted(missing)}")

    df = df.copy()
    df["cmd_profondeur"] = pd.to_numeric(df["cmd_profondeur"], errors="coerce")
    df["cmd_thrust"] = pd.to_numeric(df["cmd_thrust"], errors="coerce")
    df["crash"] = pd.to_numeric(df["crash"], errors="coerce")
    df["final_altitude"] = pd.to_numeric(df["final_altitude"], errors="coerce")
    df = df.dropna(subset=["cmd_profondeur", "cmd_thrust", "crash"])

    crash_map = df.pivot_table(
        index="cmd_profondeur",
        columns="cmd_thrust",
        values="crash",
        aggfunc="max",
    ).sort_index(axis=0).sort_index(axis=1)

    final_alt_map = df[df["crash"] == 0].pivot_table(
        index="cmd_profondeur",
        columns="cmd_thrust",
        values="final_altitude",
        aggfunc="mean",
    ).reindex(index=crash_map.index, columns=crash_map.columns)

    y_values = crash_map.index.to_numpy(dtype=float)
    x_values = crash_map.columns.to_numpy(dtype=float)
    crash_values = crash_map.to_numpy(dtype=float)
    final_alt_values = final_alt_map.to_numpy(dtype=float)

    fig, ax = plt.subplots(figsize=(10, 6))

    non_crash_mask = (crash_values == 0) & np.isfinite(final_alt_values)
    if np.any(non_crash_mask):
        alt_min = float(np.nanmin(final_alt_values[non_crash_mask]))
        alt_max = float(np.nanmax(final_alt_values[non_crash_mask]))
        if np.isclose(alt_min, alt_max):
            alt_max = alt_min + 1.0
    else:
        alt_min, alt_max = 0.0, 1.0

    altitude_cmap = cm.get_cmap("viridis")
    norm = mcolors.Normalize(vmin=alt_min, vmax=alt_max)

    rgb = np.ones((len(y_values), len(x_values), 3), dtype=float)
    rgb[:] = np.array([0.85, 0.85, 0.85], dtype=float)  # no data: gray

    if np.any(non_crash_mask):
        rgb[non_crash_mask] = altitude_cmap(norm(final_alt_values[non_crash_mask]))[:, :3]

    crash_mask = crash_values >= 1
    rgb[crash_mask] = np.array([0.0, 0.0, 0.0], dtype=float)  # crash: black

    im = ax.imshow(
        rgb,
        origin="lower",
        aspect="auto",
    )

    ax.set_title(f"Heatmap crash/final altitude — {duree_batch} s — {csv_path.name}")
    ax.set_xlabel("cmd_thrust")
    ax.set_ylabel("cmd_p")

    ax.set_xticks(np.arange(len(x_values)))
    ax.set_yticks(np.arange(len(y_values)))
    ax.set_xticklabels([f"{v:.2f}" for v in x_values], rotation=45, ha="right")
    ax.set_yticklabels([f"{v:.2f}" for v in y_values])

    sm = cm.ScalarMappable(norm=norm, cmap=altitude_cmap)
    sm.set_array([])
    cbar = fig.colorbar(sm, ax=ax)
    cbar.set_label("Final altitude (no crash)")

    legend_items = [
        Patch(facecolor="#000000", edgecolor="white", label="Crash"),
        Patch(facecolor="#d9d9d9", edgecolor="black", label="No data"),
    ]
    ax.legend(handles=legend_items, loc="upper left")

    plt.tight_layout()
    fig.savefig(output_plot_dir / "heatmap_crash.png", dpi=300, bbox_inches="tight")
    plt.show()


if __name__ == "__main__":
    main()
