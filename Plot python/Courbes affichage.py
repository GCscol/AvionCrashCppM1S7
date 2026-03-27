import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Charger le CSV
df = pd.read_csv(r"C:\Users\Matteo de Toma\Desktop\Magistère de Physique\M1\Projet Informatique\Projet info Hugo\simulation_full.csv")

n = len(df)

# Accès aux colonnes
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
cmd = df['cmd_profondeur']
alpha = df['AoA_deg']  # angle d'attaque

t1,t2 = 110, 125

Poids = 140178.9 * 9.8

# Trajectoire xOz
plt.figure()
plt.plot(x, z, label="Trajectoire xOz")
plt.plot(x, np.zeros_like(x), color='red', linestyle='--')
plt.xlabel("x [m]")
plt.ylabel("Altitude z [m]")
plt.title("Trajectoire de l'avion")
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
plt.figure()
plt.plot(t, pitch*180/np.pi, label="Pitch [deg]")
plt.plot(t, alpha, label="AoA [deg]")
plt.plot(t,20*cmd, label = "Commande")
#plt.plot([t1]*len(t), np.linspace(-50, 100, len(t)), color='green', linestyle='--', label='x=t1 m')  # ligne verticale
#plt.plot([t2]*len(t), np.linspace(-50, 100, len(t)), color='green', linestyle='--', label='x=155 m')  # ligne verticale
plt.xlabel("Temps [s]")
plt.ylabel("Angle [deg]")
plt.legend()
plt.show()



# portance
plt.figure()
plt.plot(t, portance, label="Portance")
plt.plot(t, [Poids for i in range(n)], label = 'Poids')
#plt.plot([t1]*len(t), np.linspace(-1e3, 2e4, len(t)), color='green', linestyle='--', label='x=t1 s')  # ligne verticale
#plt.plot([t2]*len(t), np.linspace(-1e3, 2e4, len(t)), color='green', linestyle='--', label='x=t2 s')  # ligne verticale
plt.xlabel("Temps [s]")
plt.ylabel("Portance [N]")
plt.legend()
plt.show()

# Vitesses
plt.figure()
plt.plot(t, np.sqrt(vx**2 + vy**2 + vz**2), label='Vitesse totale')
plt.plot(t, vx, label='vx')
plt.plot(t, vy, label='vy')
plt.plot(t, vz, label='vz')
plt.xlabel("Temps [s]")
plt.ylabel("Vitesse [m/s]")
plt.title("Vitesse de l'avion")
plt.legend()
plt.show()
