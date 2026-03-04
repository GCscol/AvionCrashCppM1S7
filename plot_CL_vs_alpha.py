import numpy as np
import matplotlib.pyplot as plt

DEG_TO_RAD = np.pi / 180.0
RAD_TO_DEG = 180.0 / np.pi

alpha0 = -0.035
alpha_tangent_deg = 13.0
alpha_peak_deg = 15.0
alpha_tangent_rad = alpha_tangent_deg * DEG_TO_RAD
alpha_peak_rad = alpha_peak_deg * DEG_TO_RAD

A = -71.6197
C_L_peak = 1.3967

slope = 5.0
elevator_auth = 0.44

def compute_CL(alpha_rad, delta_p):
    if alpha_rad <= alpha_tangent_rad:
        C_L = slope * (alpha_rad - alpha0) + elevator_auth * delta_p
    else:
        alpha_offset = alpha_rad - alpha_peak_rad
        parabolic = A * alpha_offset * alpha_offset + C_L_peak + elevator_auth * delta_p
        C_L = max(0.0, parabolic)
    return C_L

alpha_deg = np.linspace(0, 30, 600)
alpha_rad = alpha_deg * DEG_TO_RAD

delta_p_values = [0.0, 0.2, -0.2]
delta_p_labels = ['δp = 0.0', 'δp = +0.2 (pitch down)', 'δp = -0.2 (pitch up)']
colors = ['blue', 'red', 'green']

fig, ax1 = plt.subplots(1, 1, figsize=(14, 8))

for delta_p, label, color in zip(delta_p_values, delta_p_labels, colors):
    C_L = np.array([compute_CL(alpha, delta_p) for alpha in alpha_rad])
    ax1.plot(alpha_deg, C_L, color=color, linewidth=2.5, label=label)

ax1.axvline(alpha_tangent_deg, color='gray', linestyle='--', alpha=0.6, linewidth=2.5, 
            label=f'Transition point (α={alpha_tangent_deg}°)')
ax1.axvline(alpha_peak_deg, color='orange', linestyle='--', alpha=0.6, linewidth=2.5,
            label=f'C_L max (α={alpha_peak_deg}°)')

ax1.text(8.5, 0.30, 'Linear region\n(attached flow)', 
         bbox=dict(boxstyle='round', facecolor='lightblue', alpha=0.7),
         fontsize=20, ha='center')
ax1.text(27, 0.2, 'Post-stall region\n(parabolic decay)', 
         bbox=dict(boxstyle='round', facecolor='lightcoral', alpha=0.7),
         fontsize=20, ha='center')

ax1.axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.3)
ax1.grid(True, alpha=0.3)
ax1.set_ylabel(r'Lift coefficient $C_L$', fontsize=16, fontweight='bold')
ax1.legend(loc='upper right', fontsize=14)
ax1.set_xlim(0, 30)
ax1.set_ylim(-0.2, 1.6)

ax1.set_xlabel('Angle of attack α (degrees)', fontsize=16, fontweight='bold')
plt.tight_layout()
plt.savefig('output_plot/CL_vs_alpha.png', dpi=150, bbox_inches='tight')
plt.show()
