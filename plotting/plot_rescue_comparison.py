import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path

from typing import Optional  # can be remove if there is compatibility issue

# Improve default font sizes for better readability
plt.rcParams.update({
    "axes.titlesize": 16,
    "axes.labelsize": 14,
    "xtick.labelsize": 12,
    "ytick.labelsize": 12,
    "legend.fontsize": 12,
})


def load_csv(path: Path) -> pd.DataFrame:
    df = pd.read_csv(path)
    if "time" not in df.columns:
        raise ValueError(f"Missing 'time' column in {path}")
    return df


def normalize_signed(series: pd.Series) -> pd.Series:
    max_abs = series.abs().max()
    if max_abs == 0:
        return series.copy()
    return series / max_abs


def normalize_positive(series: pd.Series) -> pd.Series:
    max_val = series.max()
    if max_val == 0:
        return series.copy()
    return series / max_val


def pick_column(df: pd.DataFrame, preferred: str, fallback: str) -> Optional[str]:  #If optionnal is not included, this line need to be replace by  : def pick_column(df: pd.DataFrame, preferred: str, fallback: str) -> str | None: 
    if preferred in df.columns:
        return preferred
    if fallback in df.columns:
        return fallback
    return None


def main() -> None:
    root = Path(__file__).resolve().parent.parent
    output_dir = root / "output_file"
    output_plot_dir = root / "output_plot"
    output_plot_dir.mkdir(parents=True, exist_ok=True)
    datasets = {
        "Strategy 3: Genetic Algorithm": output_dir / "strategy3_genetic_algorithm.csv",
        "Strategy 2: Simultaneous": output_dir / "strategy2_simultaneous.csv",
        "Strategy 1: Profile first": output_dir / "strategy1_profile_first.csv",
        "Strategy 0: Thrust first": output_dir / "strategy0_thrust_first.csv",
        "No rescue": output_dir / "baseline_no_rescue.csv",
    }
    data = {label: load_csv(path) for label, path in datasets.items()}
    fig, axes = plt.subplots(4, 1, figsize=(11, 8), sharex=True)

    # Altitude comparison for all curves
    for label, df in data.items():
        if "z" in df.columns:
            axes[0].plot(df["time"], df["z"], label=label, linewidth=1.6)
    axes[0].set_ylabel("Altitude (m)")
    axes[0].set_title("Comparison of rescue strategies", fontsize=16)
    axes[0].grid(True, alpha=0.3)

    # Angle of attack comparison
    alpha_candidates = ["alpha", "alpha_deg", "alpha_rad", "angle_of_attack", "AoA", "alpha(deg)"]
    for label, df in data.items():
        alpha_col = None
        for cand in alpha_candidates:
            if cand in df.columns:
                alpha_col = cand
                break
        if alpha_col is None:
            for col in df.columns:
                if col.lower().startswith("alpha") or "angle" in col.lower() and "attack" in col.lower():
                    alpha_col = col
                    break
        if alpha_col is not None:
            vals = df[alpha_col].copy()
            if "rad" in alpha_col.lower() or vals.abs().max() <= 0.5:
                vals = np.degrees(vals)
            axes[1].plot(df["time"], vals, label=label, linewidth=1.4)
    axes[1].set_ylabel(r"$\alpha$ (deg)")
    axes[1].grid(True, alpha=0.3)

    # Normalized thrust/traction comparison
    for label, df in data.items():
        thrust_col = pick_column(df, "cmd_thrust", "traction")
        if thrust_col is not None:
            thrust_norm = normalize_positive(df[thrust_col])
            axes[2].plot(df["time"], thrust_norm, label=label, linewidth=1.4)
    axes[2].set_ylabel("cmd_T")
    axes[2].grid(True, alpha=0.3)

    # Elevator command comparison
    for label, df in data.items():
        if "cmd_profondeur" in df.columns:
            axes[3].plot(df["time"], df["cmd_profondeur"], label=label, linewidth=1.4)
    axes[3].set_xlabel("Time (s)")
    axes[3].set_ylabel("cmd_p")
    axes[3].grid(True, alpha=0.3)

    # --- Single shared legend at the bottom ---
    handles, labels = axes[0].get_legend_handles_labels()
    fig.legend(
        handles, labels,
        loc="lower center",
        ncol=3,
        bbox_to_anchor=(0.5, 0.0),
        frameon=True,
        fontsize=12,
    )

    plt.tight_layout(rect=[0, 0.08, 1, 0.97])  # bottom margin reserved for legend
    fig.savefig(output_plot_dir / "rescue_comparison.png", dpi=300, bbox_inches="tight")
    plt.show()

if __name__ == "__main__":
    main()
