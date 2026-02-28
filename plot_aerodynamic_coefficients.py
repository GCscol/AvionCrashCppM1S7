import numpy as np
import matplotlib.pyplot as plt

# Constants
DEG_TO_RAD = np.pi / 180.0
RAD_TO_DEG = 180.0 / np.pi

# Aerodynamic model parameters
alpha0 = -0.035  # zero-lift angle (rad)
alpha_tangent_deg = 13.0
alpha_peak_deg = 15.0
alpha_tangent_rad = alpha_tangent_deg * DEG_TO_RAD
alpha_peak_rad = alpha_peak_deg * DEG_TO_RAD

# Parabola parameters
A = -71.43
C_L_peak = 1.221

# Linear parameters
slope = 5.0
elevator_auth = 0.44

# Aerodynamic geometry (A330-like)
l = 7.5  # chord length (m)
m = 140000  # mass (kg)
g = 9.81

# Create alpha range
alpha_deg = np.linspace(-20, 40, 600)
alpha_rad = alpha_deg * DEG_TO_RAD

# Initialize arrays
C_L = np.zeros_like(alpha_rad)
C_D = np.zeros_like(alpha_rad)
C_m = np.zeros_like(alpha_rad)

# Compute coefficients
for i, alpha in enumerate(alpha_rad):
    # Compute C_L (parabolic stall model)
    if alpha <= alpha_tangent_rad:
        # Attached flow (linear)
        C_L[i] = slope * (alpha - alpha0) + elevator_auth * 0.0  # delta_p = 0
    else:
        # Post-stall (parabolic)
        alpha_offset = alpha - alpha_peak_rad
        parabolic = A * alpha_offset * alpha_offset + C_L_peak
        C_L[i] = max(0.0, parabolic)
    
    # Compute C_D (induced drag + zero-lift drag)
    C_D[i] = 0.0175 + 0.055 * C_L[i] * C_L[i]
    
    # Compute C_m (pitching moment)
    # omega = 0 (no angular velocity), vitesse = 100 m/s (cruise speed)
    vitesse = 100.0
    delta_p = 0.0
    omega = 0.0
    vitesse_safe = max(vitesse, 1.0)
    C_m[i] = -0.1 - (alpha - alpha0) - 1.46 * delta_p - 12.0 * omega * l / vitesse_safe

# Create figure with 3 subplots
fig, axes = plt.subplots(3, 1, figsize=(12, 10))

# Plot 1: C_L vs alpha
axes[0].plot(alpha_deg, C_L, 'b-', linewidth=2.5, label='C_L (parabolic stall)')
axes[0].axvline(alpha_tangent_deg, color='green', linestyle='--', alpha=0.6, linewidth=1.5, label=f'Transition (13°)')
axes[0].axvline(alpha_peak_deg, color='orange', linestyle='--', alpha=0.6, linewidth=1.5, label=f'Peak (15°)')
axes[0].axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.3)
axes[0].fill_between(alpha_deg, 0, C_L, alpha=0.2, color='blue')
axes[0].grid(True, alpha=0.3)
axes[0].set_ylabel('C_L (Lift Coefficient)', fontsize=11, fontweight='bold')
axes[0].set_title('Aerodynamic Coefficients vs Angle of Attack (Linear Model)', fontsize=13, fontweight='bold')
axes[0].legend(loc='upper right', fontsize=10)
axes[0].set_xlim(-20, 40)
axes[0].set_ylim(-0.3, 1.5)

# Plot 2: C_D vs alpha
axes[1].plot(alpha_deg, C_D, 'r-', linewidth=2.5, label='C_D (parabolic)')
axes[1].axvline(alpha_tangent_deg, color='green', linestyle='--', alpha=0.6, linewidth=1.5)
axes[1].axvline(alpha_peak_deg, color='orange', linestyle='--', alpha=0.6, linewidth=1.5)
axes[1].axhline(0, color='black', linestyle='-', linewidth=0.5, alpha=0.3)
axes[1].fill_between(alpha_deg, 0, C_D, alpha=0.2, color='red')
axes[1].grid(True, alpha=0.3)
axes[1].set_ylabel('C_D (Drag Coefficient)', fontsize=11, fontweight='bold')
axes[1].legend(loc='upper right', fontsize=10)
axes[1].set_xlim(-20, 40)
axes[1].set_ylim(0, 0.3)

# Plot 3: C_m vs alpha
axes[2].plot(alpha_deg, C_m, 'purple', linewidth=2.5, label='C_m (pitching moment)')
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
plt.show()
#plt.savefig('c:/Users/hugoa/OneDrive/Desktop/AvionCrashCppM1S7/output/aerodynamic_coefficients.png', dpi=150, bbox_inches='tight')
print("Plot saved to: output/aerodynamic_coefficients.png")

# Print some key values
print("\n" + "="*70)
print("AERODYNAMIC COEFFICIENTS (Linear Model with Parabolic Stall)")
print("="*70)
print(f"\nModel parameters:")
print(f"  - Zero-lift angle: α₀ = {alpha0*RAD_TO_DEG:.2f}°")
print(f"  - Linear region: α ≤ {alpha_tangent_deg}°")
print(f"  - Parabolic region: α > {alpha_tangent_deg}°")
print(f"  - Peak lift at: α = {alpha_peak_deg}° with C_L = {C_L_peak:.4f}")
print(f"  - Lift curve slope: {slope:.1f} /rad")
print(f"  - Elevator authority: {elevator_auth:.2f} /rad")

print(f"\nKey angle of attack values:")
idx_crl_max = np.argmax(C_L)
print(f"  - Maximum C_L = {C_L[idx_crl_max]:.4f} at α = {alpha_deg[idx_crl_max]:.2f}°")

idx_crl_zero = np.where((C_L[alpha_deg > 15] <= 0.01))[0]
if len(idx_crl_zero) > 0:
    alpha_crl_zero = alpha_deg[alpha_deg > 15][idx_crl_zero[0]]
    print(f"  - C_L ≈ 0 at α ≈ {alpha_crl_zero:.2f}°")

idx_crd_max = np.argmax(C_D)
print(f"  - Maximum C_D = {C_D[idx_crd_max]:.4f} at α = {alpha_deg[idx_crd_max]:.2f}°")

idx_cm_stall = np.argmin(C_m[alpha_deg > 13])
alpha_cm_stall = alpha_deg[alpha_deg > 13][idx_cm_stall]
cm_stall = C_m[alpha_deg > 13][idx_cm_stall]
print(f"  - Most negative C_m = {cm_stall:.4f} at α = {alpha_cm_stall:.2f}°")

print("\n" + "="*70)
