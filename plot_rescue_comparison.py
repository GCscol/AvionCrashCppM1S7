import pandas as pd
import matplotlib.pyplot as plt
from pathlib import Path


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


def pick_column(df: pd.DataFrame, preferred: str, fallback: str) -> str | None:
    if preferred in df.columns:
        return preferred
    if fallback in df.columns:
        return fallback
    return None


def main() -> None:
    root = Path(__file__).resolve().parent
    output_dir = root / "output_file"
    output_plot_dir = root / "output_plot"
    output_plot_dir.mkdir(parents=True, exist_ok=True)

    datasets = {
        "No rescue": output_dir / "baseline_no_rescue.csv",
        "Strategy 0: Thrust first": output_dir / "strategy0_thrust_first.csv",
        "Strategy 1: Profile first": output_dir / "strategy1_profile_first.csv",
        "Strategy 2: Simultaneous": output_dir / "strategy2_simultaneous.csv",
    }

    data = {label: load_csv(path) for label, path in datasets.items()}

    fig, axes = plt.subplots(3, 1, figsize=(11, 10), sharex=True)

    # Altitude comparison for all curves
    for label, df in data.items():
        if "z" in df.columns:
            axes[0].plot(df["time"], df["z"], label=label, linewidth=1.6)
    axes[0].set_ylabel("Altitude (m)")
    axes[0].set_title("Altitude vs Time (No rescue vs 3 rescue strategies)")
    axes[0].grid(True, alpha=0.3)
    axes[0].legend()

    # Normalized thrust/traction comparison
    for label, df in data.items():
        thrust_col = pick_column(df, "cmd_thrust", "traction")
        if thrust_col is not None:
            thrust_norm = normalize_positive(df[thrust_col])
            axes[1].plot(df["time"], thrust_norm, label=label, linewidth=1.4)
    axes[1].set_ylabel("Normalized Thrust")
    axes[1].set_title("Normalized Thrust vs Time")
    axes[1].grid(True, alpha=0.3)
    axes[1].legend()

    # Elevator command comparison
    for label, df in data.items():
        if "cmd_profondeur" in df.columns:
            axes[2].plot(df["time"], df["cmd_profondeur"], label=label, linewidth=1.4)
    axes[2].set_xlabel("Time (s)")
    axes[2].set_ylabel("Normalized cmd_profondeur")
    axes[2].set_title("cmd_profondeur vs Time")
    axes[2].grid(True, alpha=0.3)
    axes[2].legend()

    plt.tight_layout()
    fig.savefig(output_plot_dir / "rescue_comparison.png", dpi=300, bbox_inches="tight")
    plt.show()


if __name__ == "__main__":
    main()
