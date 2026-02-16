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


def main() -> None:
    root = Path(__file__).resolve().parent
    rescue_path = root / "output" / "scenario1_progressif.csv"
    baseline_path = root / "output" / "baseline_no_rescue.csv"

    df_rescue = load_csv(rescue_path)
    df_base = load_csv(baseline_path)

    fig, axes = plt.subplots(3, 1, figsize=(11, 10), sharex=True)

    # Altitude comparison
    axes[0].plot(df_rescue["time"], df_rescue["z"], label="Rescue", linewidth=1.6)
    axes[0].plot(df_base["time"], df_base["z"], label="No rescue", linewidth=1.4, alpha=0.8)
    axes[0].set_ylabel("Altitude (m)")
    axes[0].set_title("Altitude vs Time")
    axes[0].grid(True, alpha=0.3)
    axes[0].legend()

    # Normalized cmd_thrust (or traction proxy)
    if "cmd_thrust" in df_rescue.columns:
        thrust_norm = normalize_positive(df_rescue["cmd_thrust"])
        axes[1].plot(df_rescue["time"], thrust_norm, label="cmd_thrust (norm)", linewidth=1.4)
    elif "traction" in df_rescue.columns:
        traction_norm = normalize_positive(df_rescue["traction"])
        axes[1].plot(df_rescue["time"], traction_norm, label="traction (norm, proxy)", linewidth=1.4)
    axes[1].set_ylabel("Normalized Thrust")
    axes[1].set_title("Normalized Thrust vs Time")
    axes[1].grid(True, alpha=0.3)
    axes[1].legend()

    # Normalized cmd_profondeur
    if "cmd_profondeur" in df_rescue.columns:
        prof_norm = df_rescue["cmd_profondeur"]
        axes[2].plot(df_rescue["time"], prof_norm, label="cmd_profondeur (norm)", linewidth=1.4)
    axes[2].set_xlabel("Time (s)")
    axes[2].set_ylabel("Normalized cmd_profondeur")
    axes[2].set_title("Normalized cmd_profondeur vs Time")
    axes[2].grid(True, alpha=0.3)
    axes[2].legend()

    plt.tight_layout()
    plt.show()


if __name__ == "__main__":
    main()
