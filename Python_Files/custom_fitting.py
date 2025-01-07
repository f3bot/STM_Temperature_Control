import numpy as np
import matplotlib.pyplot as plt
import pandas as pd

def temperature_model(t, k, T0, T):
    return k * (1 - np.exp(-(t - T0) / T)) * (t > T0)

k = 23.27
T0 = .5
T = 290
print("Parametry Transmitancji")
print("K = ", k)
print("T0 = ", T0)
print("T = ", T)
timeS = np.linspace(0, 3500,3500)  

temperature = temperature_model(timeS, k, T0, T)
data = pd.read_csv("proszedzialaj.csv", encoding='latin1')

data['Time (s)'] = data['Timestamp (ms)'] / 1000

time = data['Time (s)']
normalized_temperature = data['Normalized Temperature (°C)']


plt.figure(figsize=(10, 6))
plt.plot(time, normalized_temperature)
plt.plot(timeS, temperature, label=f'Temperature Model: k={k}, T0={T0}, T={T}', color='blue')
plt.xlabel('Time (s)')
plt.ylabel('Temperature (°C)')
plt.title('User-Defined Temperature Model')
plt.legend()
plt.grid(True)
plt.show()
