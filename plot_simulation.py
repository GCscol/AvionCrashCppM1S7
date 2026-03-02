import sys
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

fn = "simulation_full.csv"

df = pd.read_csv(fn)

required = ["time", "x", "z", "pitch", "Fx", "Fz", "AoA_deg", "M_pitch", "vx", "vz"]
for c in required:
    if c not in df.columns:
        raise SystemExit(f"Colonne manquante dans {fn} : {c}")

t = df["time"]

fig, axes = plt.subplots(3, 1, figsize=(10, 16), sharex=True)

# Velocities vx, vz, and absolute velocity vs time
v_abs = np.sqrt(df["vx"]**2 + df["vz"]**2)

axes[0].plot(t, df["vx"], label="vx (horizontal)", linewidth=2)
axes[0].plot(t, df["vz"], label="vz (vertical)", linewidth=2)
axes[0].plot(t, v_abs, label="V (vitesse absolue)", color="black", linewidth=2.5, linestyle="-")
axes[0].axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.3)
axes[0].set_ylabel("Vitesse (m/s)")
axes[0].legend(loc='best')
axes[0].grid(True, alpha=0.3)

# Angles: pitch, alpha, and flight path angle (gamma)
gamma_rad = np.arctan2(df["vz"], df["vx"])
gamma_deg = gamma_rad * 180 / np.pi

axes[1].plot(t, df["pitch"]*180/np.pi, label="pitch (θ)", color="C2", linewidth=2)
axes[1].plot(t, df["AoA_deg"], label="incidence (α)", color="C3", linewidth=2)
axes[1].plot(t, gamma_deg, label="angle de pente (γ)", color="purple", linewidth=2, linestyle="--")
axes[1].axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.3)
axes[1].set_ylabel("Angle (deg)")
axes[1].legend(loc='best')
axes[1].grid(True, alpha=0.3)

# # Pitching moment vs temps
# axes[2].plot(t, df["M_pitch"], label="M_pitch", color="C2")
# axes[2].set_ylabel("M_pitch (N.m)")
# axes[2].legend()
# axes[2].grid(True)

# Altitude vs temps
axes[2].plot(t, df["z"], label="z (altitude)")
axes[2].set_ylabel("Position (m)")
axes[2].legend()
axes[2].grid(True)

plt.tight_layout()
print("✓ Plot saved to output/simulation_analysis.png")
plt.show()
