import numpy as np
import matplotlib.pyplot as plt


data = np.loadtxt(r"Code/rk4_1.res")

t,x,y,theta, vx, vy = data[:,0], data[:,1], data[:,2], data[:,3], data[:,4], data[:,5]

ax1 = plt.subplot(2,1,1)
ax1.set_xlabel("Temps [s]")
ax1.set_ylabel("Distance [m]")
ax1.plot(t,x, label="Position")
ax1.plot(t,y, label="Altitude")
ax1.legend()
ax2 = plt.subplot(2,1,2)
ax2.set_xlabel("Temps [s]")
ax2.set_ylabel(r"Vitesses [$m\cdot s^{-1}$]")
ax2.plot(t,vx, label="vx")
ax2.plot(t,vy, label="vy")
ax2.legend()
plt.show()
