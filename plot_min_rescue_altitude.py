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


def plot_optimizer(ax: plt.Axes, root: Path) -> bool:
    csv_path = root / "output_file" / "min_rescue_altitude_opt_results.csv"
    if not csv_path.exists():
        return False

    df = pd.read_csv(csv_path)
    df_ok  = df[df["success"] == 1].sort_values("activation_altitude")
    df_nok = df[df["success"] == 0].sort_values("activation_altitude")

    if not df_ok.empty:
        ax.plot(
            df_ok["activation_altitude"],
            df_ok["recovery_altitude"],
            marker="D", linewidth=2.0, color="#2ca02c",
            label="Optimizer (success)",
        )
    if not df_nok.empty:
        ax.scatter(
            df_nok["activation_altitude"],
            df_nok["recovery_altitude"],
            marker="x", color="red", s=80, zorder=5,
            label="Optimizer (crash)",
        )

    return (not df_ok.empty) or (not df_nok.empty)


def main() -> None:
    root = Path(__file__).resolve().parent
    output_plot_dir = root / "output_plot"
    output_plot_dir.mkdir(parents=True, exist_ok=True)
    fig, ax = plt.subplots(figsize=(11, 7))

    has_fixed = plot_fixed_strategies(ax, root)
    has_opt = plot_optimizer(ax, root)

    ax.set_title("Recovery altitude vs activation altitude (with and without optimization)")
    ax.set_xlabel("Rescue activation altitude (m)")
    ax.set_ylabel("Recovery altitude (m)")
    ax.grid(True, alpha=0.3)

    if has_fixed or has_opt:
        ax.legend()
    else:
        ax.text(
            0.5,
            0.5,
            "No data found.\nRun MIN_RESCUE_ALTITUDE and/or MIN_RESCUE_ALT_OPT first.",
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
