import pandas as pd
import matplotlib.pyplot as plt
import numpy as np

g = 9.81
masse_avion = 140178.9

df = pd.read_csv("simulation_full.csv")

"""
 csv << "time,"
        << "x,y,z,vx,vy,vz,"
        << "roll,pitch,yaw,"
        << "M_pitch, I_pitch, omega_pitch,"
        << "Fx,Fy,Fz,portance,trainee,traction,"
        << "Cl,Cd,Cm,"
        << "speed,AoA_deg,cmd_profondeur,alpha,delta_profondeur,n_factor\n";
"""
print("max pitch :", df["pitch"].max() , "min pitch :", df["pitch"].min())
print("max omega_pitch :", df["omega_pitch"].max() , "min omega_pitch :", df["omega_pitch"].min())
print("max roll :", df["roll"].max() , "min roll :", df["roll"].min())
print("max yaw :", df["yaw"].max() , "min yaw :", df["yaw"].min())

df["Ecin"] = 0.5 * masse_avion * (df["speed"]**2)
df["Erot"] = 0.5 * df["I_pitch"] * (df["omega_pitch"]**2)
df["Epot"] = masse_avion * g * df["z"]


Fx = df["Fx"].values
Fy = df["Fy"].values
Fz = df["Fz"].values
x = df["x"].values
y = df["y"].values
z = df["z"].values

n = len(df)
Wx = np.zeros(n)
Wy = np.zeros(n)
Wz = np.zeros(n)

Wx[0] = Fx[0] * (x[1] - x[0]) * 0.5
Wy[0] = Fy[0] * (y[1] - y[0]) * 0.5
Wz[0] = Fz[0] * (z[1] - z[0]) * 0.5

Wx[-1] = Fx[-1] * (x[-1] - x[-2]) * 0.5
Wy[-1] = Fy[-1] * (y[-1] - y[-2]) * 0.5
Wz[-1] = Fz[-1] * (z[-1] - z[-2]) * 0.5

Wx[1:-1] = Fx[1:-1] * (x[2:] - x[:-2]) * 0.5
Wy[1:-1] = Fy[1:-1] * (y[2:] - y[:-2]) * 0.5
Wz[1:-1] = Fz[1:-1] * (z[2:] - z[:-2]) * 0.5

df["Wx"] = Wx
df["Wy"] = Wy
df["Wz"] = Wz

df["DeltaNRJcin%"] = ( df["Ecin"] + df["Erot"] + df["Epot"] - df["Wx"] - df["Wy"] - df["Wz"] ) * 100/ ( df["Ecin"][0] + df["Erot"][0] + df["Epot"][0] )


fig, (ax1, ax3) = plt.subplots(1, 2, figsize=(16, 6))

ax1.plot(df["time"], df["DeltaNRJcin%"], label="DeltaNRJmec%", linewidth=2, color='darkblue')
ax1.set_xlabel("Temps (s)")
ax1.set_ylabel("Énergie (%)")
ax1.set_title("Variation d'énergie mécanique (en (%) de Ecin Erot et Epot initiales)")
ax1.legend()
ax1.grid(True)


ax3.plot(df["time"], df["Epot"], label="Epot", color='cyan', linewidth=1)
ax3.plot(df["time"], df["Ecin"], label="Ecin", color='orange', linewidth=1)
ax3.set_xlabel("Temps (s)")
ax3.set_ylabel("Énergie potentielle et cinétique (J)", color='black')
ax3.tick_params(axis='y')
ax3.legend(loc='upper left')
ax3.grid(True, alpha=0.3)

ax4 = ax3.twinx()
ax4.plot(df["time"], df["Erot"], label="Erot", color='blue', linewidth=1, linestyle='--')
ax4.plot(df["time"], df["Wx"], label="Wx", color='green', linewidth=1, linestyle=':')
ax4.plot(df["time"], df["Wy"], label="Wy", color='red', linewidth=1, linestyle=':')
ax4.plot(df["time"], df["Wz"], label="Wz", color='pink', linewidth=1, linestyle=':')
ax4.set_ylabel("Énergie de rotation et W (J)", color='black')
ax4.tick_params(axis='y')
ax4.legend(loc='upper right')

ax3.set_title("Énergies potentielle, cinétique, rotationnelle et travail")

plt.tight_layout()
plt.show()

