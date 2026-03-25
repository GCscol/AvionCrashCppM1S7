import sys
from pathlib import Path

import pandas as pd
import matplotlib.pyplot as plt


def resolve_csv_path(root: Path, cli_arg: str | None) -> Path:
    if cli_arg:
        return Path(cli_arg)

    default_csv = root / "output_file" / "Phugoid.csv"
    if default_csv.exists():
        return default_csv

    fallback_csv = root / "output_file" / "test_simulation_full.csv"
    if fallback_csv.exists():
        return fallback_csv

    raise FileNotFoundError(
        "CSV introuvable. Attendu: output_file/Phugoid.csv "
        "(ou fallback output_file/test_simulation_full.csv)."
    )


def main() -> None:
    root = Path(__file__).resolve().parent
    output_plot_dir = root / "output_plot"
    output_plot_dir.mkdir(parents=True, exist_ok=True)

    csv_path = resolve_csv_path(root, sys.argv[1] if len(sys.argv) > 1 else None)
    df = pd.read_csv(csv_path)

    required = {"x", "z"}
    missing = required - set(df.columns)
    if missing:
        raise ValueError(f"Colonnes manquantes dans {csv_path}: {sorted(missing)}")

    x = df["x"]
    z = df["z"]

    fig, ax = plt.subplots(figsize=(10, 6))
    ax.plot(x, z, label="Trajectoire xOz")
    ax.set_xlabel("x [m]")
    ax.set_ylabel("Altitude z [m]")
    ax.set_title("Path of the plane")
    ax.legend()
    ax.grid(True, alpha=0.3)

    plt.tight_layout()
    fig.savefig(output_plot_dir / "phugoide.png", dpi=300, bbox_inches="tight")
    plt.show()


if __name__ == "__main__":
    main()
