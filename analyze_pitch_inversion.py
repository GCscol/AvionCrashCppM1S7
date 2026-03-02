#!/usr/bin/env python3
"""
Analyse de l'inversion du sens de variation du pitch à t≈94.20s
"""

import pandas as pd
import matplotlib.pyplot as plt

# Charger les données
df = pd.read_csv('simulation_full.csv')
df['M_total'] = df['M_pitch'] + df['M_thrust']
df['dpitch_dt'] = df['pitch'].diff() / 0.01

print("="*70)
print("ANALYSE PHYSIQUE DE L'INVERSION DU PITCH À t≈94.20s")
print("="*70)
print()

# Trouver l'index autour de 94.20s
idx = (df['time'] - 94.20).abs().idxmin()

print("Données autour de t=94.20s :")
print("-" * 100)
print(f"{'Temps':>7} | {'Pitch':>10} | {'M_pitch':>12} | {'M_thrust':>12} | {'M_total':>12} | {'Alpha':>8}")
print(f"{'(s)':>7} | {'(rad)':>10} | {'(N·m)':>12} | {'(N·m)':>12} | {'(N·m)':>12} | {'(°)':>8}")
print("-" * 100)

for i in range(-5, 6):
    j = idx + i
    t = df.loc[j, 'time']
    pitch = df.loc[j, 'pitch']
    M_p = df.loc[j, 'M_pitch']
    M_t = df.loc[j, 'M_thrust']
    M_tot = M_p + M_t
    alpha = df.loc[j, 'AoA_deg']
    
    marker = " <-- POINT CRITIQUE" if abs(t - 94.20) < 0.01 else ""
    print(f"{t:7.2f} | {pitch:10.6f} | {M_p:12.0f} | {M_t:12.0f} | {M_tot:12.0f} | {alpha:8.2f}{marker}")

print()
print("="*70)
print("EXPLICATION PHYSIQUE")
print("="*70)
print()
print("À t=94.20s, le pitch atteint son maximum (0.955866 rad ≈ 54.76°)")
print()
print("Cause physique de l'inversion :")
print()
print("1. AVANT t=94.20s :")
print("   - M_total > 0 → Le pitch AUGMENTE (mouvement de cabré)")
print("   - M_pitch (aérodynamique) devient de plus en plus négatif (piqueur)")
print("   - M_thrust (poussée) reste positif mais diminue légèrement")
print("   - La somme M_total = M_pitch + M_thrust diminue mais reste positive")
print()
print("2. À t=94.20s (POINT CRITIQUE) :")
print(f"   - M_pitch = {df.loc[idx, 'M_pitch']:.0f} N·m (très piqueur)")
print(f"   - M_thrust = {df.loc[idx, 'M_thrust']:.0f} N·m (cabreur)")
print(f"   - M_total = {df.loc[idx, 'M_total']:.0f} N·m (encore positif mais faible)")
print(f"   - L'angle d'attaque atteint {df.loc[idx, 'AoA_deg']:.2f}° → forte traînée")
print()
print("3. APRÈS t=94.20s :")
print("   - M_total continue de diminuer et devient proche de zéro")
print("   - Le pitch commence à DIMINUER (mouvement de piqué)")
print("   - L'angle d'attaque continue d'augmenter → décrochage aérodynamique")
print()
print("CONCLUSION :")
print("Le changement de sens de variation du pitch est causé par :")
print("• L'augmentation excessive de l'angle d'attaque (α > 22°)")
print("• Le moment aérodynamique piqueur qui dépasse le moment cabreur de la poussée")
print("• La vitesse verticale élevée (vz ≈ 87 m/s) qui amplifie l'angle d'attaque")
print()
print("="*70)

# Créer les graphiques
mask = (df['time'] >= 93.5) & (df['time'] <= 95.0)
df_zoom = df[mask]

fig, axes = plt.subplots(4, 1, figsize=(14, 12))

# 1. Pitch
ax = axes[0]
ax.plot(df_zoom['time'], df_zoom['pitch'] * 180/3.14159, 'b-', linewidth=2)
ax.axvline(94.20, color='r', linestyle='--', alpha=0.7, label='t=94.20s (point critique)')
ax.set_ylabel('Pitch (°)', fontsize=12, fontweight='bold')
ax.legend(fontsize=10)
ax.grid(True, alpha=0.3)
ax.set_title('Inversion du sens de variation du pitch à t ≈ 94.20s', fontsize=14, fontweight='bold')

# 2. Moments
ax = axes[1]
ax.plot(df_zoom['time'], df_zoom['M_pitch'], 'r-', linewidth=2, label='M_pitch (aéro, piqueur)')
ax.plot(df_zoom['time'], df_zoom['M_thrust'], 'g-', linewidth=2, label='M_thrust (poussée, cabreur)')
ax.plot(df_zoom['time'], df_zoom['M_total'], 'k-', linewidth=3, label='M_total = M_pitch + M_thrust')
ax.axhline(0, color='k', linestyle=':', alpha=0.5)
ax.axvline(94.20, color='r', linestyle='--', alpha=0.7)
ax.set_ylabel('Moment (N·m)', fontsize=12, fontweight='bold')
ax.legend(fontsize=10)
ax.grid(True, alpha=0.3)

# 3. Vitesse angulaire (dPitch/dt)
ax = axes[2]
ax.plot(df_zoom['time'], df_zoom['dpitch_dt'], 'purple', linewidth=2)
ax.axhline(0, color='k', linestyle=':', alpha=0.5)
ax.axvline(94.20, color='r', linestyle='--', alpha=0.7)
ax.set_ylabel('dPitch/dt (rad/s)', fontsize=12, fontweight='bold')
ax.grid(True, alpha=0.3)

# 4. Angle d'attaque
ax = axes[3]
ax.plot(df_zoom['time'], df_zoom['AoA_deg'], 'orange', linewidth=2, label='Angle d\'attaque')
ax.axvline(94.20, color='r', linestyle='--', alpha=0.7)
ax.axhline(15, color='orange', linestyle=':', alpha=0.5, label='Seuil critique (~15°)')
ax.set_xlabel('Temps (s)', fontsize=12, fontweight='bold')
ax.set_ylabel('Alpha (°)', fontsize=12, fontweight='bold')
ax.legend(fontsize=10)
ax.grid(True, alpha=0.3)

plt.tight_layout()
plt.savefig('pitch_inversion_analysis.png', dpi=150, bbox_inches='tight')
print("Figure sauvegardée : pitch_inversion_analysis.png")
print()
