import numpy as np
import matplotlib.pyplot as plt


data = np.loadtxt("simulation_full.csv", skiprows=1, delimiter=",")


t,x,y,z,vx, vy, vz,roll, pitch, yaw, M_pitch, I_pitch, omega_pitch = data[:,0], data[:,1], data[:,2], data[:,3], data[:,4], data[:,5], data[:,6], data[:,7], data[:,8], data[:,9], data[:,10], data[:,11], data[:,12]

fig, ((ax1, ax3), (ax5, ax6)) = plt.subplots(2, 2, figsize=(16, 6))

ax1 = plt.subplot(2,2,1)
ax1.set_xlabel("Temps [s]")
ax1.set_ylabel("Distance [m]", color="blue")
ax1.plot(t,x, label="Position",color="blue")
ax1.plot(t,z, label="Altitude",color="orange")
ax1.legend(loc='upper left')
ax1.grid(True)

ax2 = ax1.twinx()
ax2.plot(t,z, label="Altitude",color="black")
ax2.set_ylabel("Distance [m]", color="black")
ax2.tick_params(axis='y')
ax2.legend(loc='upper right')

ax3 = plt.subplot(2,2,2)
ax3.set_xlabel("Temps [s]")
ax3.set_ylabel(r"Vitesses [$m\cdot s^{-1}$] (vx +(vx+vz))", color="blue")
ax3.plot(t,vx, label="vx",color="blue")
ax3.plot(t,vx+vz, label="vx+vz",color="orange")
ax3.tick_params(axis='y')
ax3.legend(loc='upper left')
ax3.grid(True)

ax4 = ax3.twinx()
ax4.plot(t,vz, label="vz",color="black")
ax4.set_ylabel(r"Vitesses [$m\cdot s^{-1}$]", color="black")
ax4.tick_params(axis='y')
ax4.legend(loc='upper right')

ax5 = plt.subplot(2,2,3)
ax5.plot(t,pitch, label="pitch",color="blue")
ax5.plot(t,roll, label="roll",color="green")
ax5.plot(t,yaw, label="yaw",color="orange")
ax5.set_xlabel("Temps [s]")
ax5.set_ylabel("angle")
ax5.legend(loc='upper left')
ax5.grid(True)

ax6 = plt.subplot(2,2,4)
ax6.plot(t,omega_pitch, label="Omega pitch",color="blue")
ax6.set_xlabel("Temps [s]")
ax6.set_ylabel("Vitesse angulaire")
ax6.legend(loc='upper left')
ax6.grid(True)

ax7 = ax6.twinx()
ax7.plot(t,M_pitch, label="M_pitch",color="black",alpha=0.3)
ax7.tick_params(axis='y')
ax7.legend(loc='upper right')

# ax6.set_xlim(0,5)
# ax5.set_xlim(0,5)
# ax3.set_xlim(0,5)
# ax1.set_xlim(0,5)

plt.tight_layout()
plt.show()
