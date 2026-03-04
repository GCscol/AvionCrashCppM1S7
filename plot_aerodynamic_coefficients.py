import numpy as np
import matplotlib.pyplot as plt

DEG_TO_RAD = np.pi / 180.0
RAD_TO_DEG = 180.0 / np.pi

alpha0 = -0.035
alpha_tangent_deg = 13.0
alpha_peak_deg = 15.0
alpha_tangent_rad = alpha_tangent_deg * DEG_TO_RAD
alpha_peak_rad = alpha_peak_deg * DEG_TO_RAD
alpha_tangent_neg_deg = -4.0
alpha_tangent_neg_rad = alpha_tangent_neg_deg * DEG_TO_RAD

A = -71.6197
C_L_peak = 1.3967

slope = 5.0
elevator_auth = 0.44
cm_alpha0 = 1.0

l = 7.5
m = 140000
g = 9.81

alpha_deg = np.linspace(-20, 40, 600)
alpha_rad = alpha_deg * DEG_TO_RAD

C_L = np.zeros_like(alpha_rad)
C_D = np.zeros_like(alpha_rad)
C_m = np.zeros_like(alpha_rad)

delta_p = 0.0  # elevator deflection
vitesse = 100.0  # cruise speed
omega = 0.0  # no angular velocity
vitesse_safe = max(vitesse, 1.0)

for i, alpha in enumerate(alpha_rad):
    if alpha <= alpha_tangent_rad:
        C_L[i] = slope * (alpha - alpha0) + elevator_auth * delta_p
    else:
        alpha_offset = alpha - alpha_peak_rad
        parabolic = A * alpha_offset * alpha_offset + C_L_peak + elevator_auth * delta_p
        C_L[i] = max(0.0, parabolic)
    
    cd0 = 0.0175
    cd_induced = 0.055
    
    if alpha <= alpha_tangent_neg_rad:
        C_L_neg = slope * (alpha_tangent_neg_rad - alpha0) + elevator_auth * delta_p
        C_D_neg = cd0 + cd_induced * C_L_neg * C_L_neg
        sin2_delta = np.sin(alpha)**2 - np.sin(alpha_tangent_neg_rad)**2
        C_D[i] = C_D_neg + 2.0 * sin2_delta
    elif alpha >= alpha_tangent_rad:
        C_L_pos = slope * (alpha_tangent_rad - alpha0) + elevator_auth * delta_p
        C_D_pos = cd0 + cd_induced * C_L_pos * C_L_pos
        sin2_delta = np.sin(alpha)**2 - np.sin(alpha_tangent_rad)**2
        C_D[i] = C_D_pos + 2.0 * sin2_delta
    else:
        C_D[i] = cd0 + cd_induced * C_L[i] * C_L[i]
    
    C_m[i] = -0.1 - cm_alpha0 * (alpha - alpha0) - 1.46 * delta_p - 12.0 * omega * l / vitesse_safe

fig, axes = plt.subplots(3, 1, figsize=(12, 10))

axes[0].plot(alpha_deg, C_L, 'b-', linewidth=2.5, label='C_L (parabolic stall)')
axes[0].axvline(alpha_tangent_neg_deg, color='purple', linestyle='--', alpha=0.6, linewidth=1.5, label=f'Transition (-4°)')
axes[0].axvline(alpha_tangent_deg, color='green', linestyle='--', alpha=0.6, linewidth=1.5, label=f'Transition (13°)')
axes[0].axvline(alpha_peak_deg, color='orange', linestyle='--', alpha=0.6, linewidth=1.5, label=f'Peak (15°)')
axes[0].axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.3)
axes[0].fill_between(alpha_deg, 0, C_L, alpha=0.2, color='blue')
axes[0].grid(True, alpha=0.3)
axes[0].set_ylabel('C_L (Lift Coefficient)', fontsize=11, fontweight='bold')
axes[0].set_title('Aerodynamic Coefficients vs Angle of Attack (ModeleLineaire)', fontsize=13, fontweight='bold')
axes[0].legend(loc='upper right', fontsize=10)
axes[0].set_xlim(-20, 40)
axes[0].set_ylim(-0.3, 1.5)

axes[1].plot(alpha_deg, C_D, 'r-', linewidth=2.5, label='C_D (induced + sin^2 transitions)')
axes[1].axvline(alpha_tangent_neg_deg, color='purple', linestyle='--', alpha=0.6, linewidth=1.5)
axes[1].axvline(alpha_tangent_deg, color='green', linestyle='--', alpha=0.6, linewidth=1.5)
axes[1].axvline(alpha_peak_deg, color='orange', linestyle='--', alpha=0.6, linewidth=1.5)
axes[1].axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.3)
axes[1].fill_between(alpha_deg, 0, C_D, alpha=0.2, color='red')
axes[1].grid(True, alpha=0.3)
axes[1].set_ylabel('C_D (Drag Coefficient)', fontsize=11, fontweight='bold')
axes[1].legend(loc='upper right', fontsize=10)
axes[1].set_xlim(-20, 40)
axes[1].set_ylim(0, 0.3)

axes[2].plot(alpha_deg, C_m, 'purple', linewidth=2.5, label='C_m (pitching moment)')
axes[2].axvline(alpha_tangent_neg_deg, color='purple', linestyle='--', alpha=0.6, linewidth=1.5)
axes[2].axvline(alpha_tangent_deg, color='green', linestyle='--', alpha=0.6, linewidth=1.5)
axes[2].axvline(alpha_peak_deg, color='orange', linestyle='--', alpha=0.6, linewidth=1.5)
axes[2].axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.5)
axes[2].fill_between(alpha_deg, 0, C_m, where=(C_m >= 0), alpha=0.2, color='purple', label='Positive (pitch up)')
axes[2].fill_between(alpha_deg, 0, C_m, where=(C_m < 0), alpha=0.2, color='orange', label='Negative (pitch down)')
axes[2].grid(True, alpha=0.3)
axes[2].set_xlabel('Angle of Attack (degrees)', fontsize=11, fontweight='bold')
axes[2].set_ylabel('C_m (Moment Coefficient)', fontsize=11, fontweight='bold')
axes[2].legend(loc='upper right', fontsize=10)
axes[2].set_xlim(-20, 40)

plt.tight_layout()
plt.savefig('output_plot/aerodynamic_coefficients.png', dpi=150, bbox_inches='tight')
plt.show()