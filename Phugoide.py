import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Charger le CSV
df = pd.read_csv(r"C:\Users\Matteo de Toma\Desktop\Magistère de Physique\M1\Projet Informatique\AvionCrashCppM1S7-main\simulation_full.csv")

n = len(df)

# Accès aux colonnes
t1,t2 = 0, 200
mask = (t1 <= df['time']) & (df['time'] <= t2)
df = df[mask]
t = df['time']
x = df['x']
y = df['y']
z = df['z']
vx = df['vx']
vy = df['vy']
vz = df['vz']
roll = df['roll']
pitch = df['pitch']
yaw = df['yaw']
M_pitch = df['M_pitch']
M_thrust = df['M_thrust']
Fx = df['Fx']
Fy = df['Fy']
Fz = df['Fz']
portance = df['portance']
trainee = df['trainee']
traction = df['traction']
Cl = df['Cl']
Cd = df['Cd']
Cm = df['Cm']

speed = df['speed']
alpha = df['AoA_deg']  # angle d'attaque

cmd = df['cmd_profondeur']
cmd_thrust = df['cmd_thrust']

delta_p = df['delta_profondeur']
n_factor = df['n_factor']


# Trajectoire xOz
plt.figure()
plt.plot(x, z, label="Trajectoire xOz")
#plt.plot(x, np.zeros_like(x), color='red', linestyle='--')
plt.xlabel("x [m]")
plt.ylabel("Altitude z [m]")
plt.title("Path of the plane")
plt.legend()
plt.show()

# Altitude en fonction du temps
plt.figure()
plt.plot(t, z, label="Altitude")

plt.xlabel("Temps [s]")
plt.ylabel("Altitude [m]")
plt.legend()
plt.show()

# Angles
