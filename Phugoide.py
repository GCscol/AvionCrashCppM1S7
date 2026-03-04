import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

# Charger le CSV
df = pd.read_csv(r"Phugoid.csv")

n = len(df)

# Accès aux colonnes
t = df['time']
x = df['x']
z = df['z']





# Trajectoire xOz
plt.figure(figsize = (10,6))
plt.plot(x, z, label="Trajectoire xOz")
#plt.plot(x, np.zeros_like(x), color='red', linestyle='--')
plt.xlabel("x [m]")
plt.ylabel("Altitude z [m]")
plt.title("Path of the plane")
plt.legend()
plt.show()


