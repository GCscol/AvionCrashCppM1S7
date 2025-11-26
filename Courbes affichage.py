import numpy as np
import matplotlib.pyplot as plt
data = np.loadtxt(r"C:\Users\Matteo de Toma\Documents\Décrochage\Projet Informatique\euler_1.res")

t,x,y,theta, vx, vy = data[:,0], data[:,1], data[:,2], data[:,3], data[:,4], data[:,5]

plt.xlabel("Temps [s]")
plt.ylabel("Distance [m]")
plt.plot(t,x, label="Position")
plt.plot(t,y, label="Altitude")
plt.legend()
plt.show()
plt.xlabel("Temps [s]")
plt.ylabel(r"Vitesses [$m\cdot s^{-1}$]")
plt.plot(t,vx, label="vx")
plt.plot(t,vy, label="vy")
plt.legend()
plt.show()
