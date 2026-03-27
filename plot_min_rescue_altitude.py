import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path


def plot_fixed_strategies(ax: plt.Axes, root: Path) -> bool:
    csv_path = root / "output_file" / "min_rescue_altitude_results.csv"
    if not csv_path.exists():
        return False

    df = pd.read_csv(csv_path)
    df_ok = df[df["success"] == 1].copy()

    styles = {
        "THRUST_FIRST":  {"label": "Strategy 0: Thrust first",  "marker": "o"},
        "PROFILE_FIRST": {"label": "Strategy 1: Profile first", "marker": "s"},
        "SIMULTANEOUS":  {"label": "Strategy 2: Simultaneous",  "marker": "^"},
    }

    if df_ok.empty:
        return False

    for strategy, group in df_ok.groupby("strategy"):
        group = group.sort_values("activation_altitude")
        style = styles.get(strategy, {"label": strategy, "marker": "o"})
        ax.plot(
            group["activation_altitude"],
            group["recovery_altitude"],
            marker=style["marker"],
            linewidth=1.8,
            label=style["label"],
        )

    return True


# Optimizer-related plotting removed per request (optimiseurAltitude)


def main() -> None:
    root = Path(__file__).resolve().parent
    output_plot_dir = root / "output_plot"
    output_plot_dir.mkdir(parents=True, exist_ok=True)
    fig, ax = plt.subplots(figsize=(11, 7))

    has_fixed = plot_fixed_strategies(ax, root)

    ax.set_title("Recovery altitude vs activation altitude")
    ax.set_xlabel("Rescue activation altitude (m)")
    ax.set_ylabel("Recovery altitude (m)")
    ax.grid(True, alpha=0.3)

    if has_fixed:
        ax.legend()
    else:
        ax.text(
            0.5,
            0.5,
            "No data found.\nRun MIN_RESCUE_ALTITUDE first.",
            ha="center",
            va="center",
            transform=ax.transAxes,
            color="gray",
        )

    plt.tight_layout()
    fig.savefig(output_plot_dir / "min_rescue_altitude_comparison.png", dpi=300, bbox_inches="tight")
    plt.show()


if __name__ == "__main__":
    main()
