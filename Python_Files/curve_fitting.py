import numpy as np
from scipy.optimize import curve_fit
from matplotlib import pyplot as plt
import pandas as pd

data = pd.read_csv("proszedzialaj.csv", encoding='latin1')

data['Time (s)'] = data['Timestamp (ms)'] / 1000

data['Normalized Temperature (°C)'] = data['Temperature (°C)'] - data['Temperature (°C)'].min()

def adjusted_temperature_model(t, k, T0, T):
    offset = k * (1 - np.exp(0))  # The value at t = T0
    return k * (1 - np.exp(-(t - T0) / T)) * (t > T0) - offset
time = data['Time (s)']
normalized_temperature = data['Normalized Temperature (°C)']

initial_guesses = [24.45, 25, 354]  # [k, T0, T]

params, covariance = curve_fit(adjusted_temperature_model, time, normalized_temperature, p0=initial_guesses)

k_fit, T0_fit, T_fit = params

fitted_temperature = adjusted_temperature_model(time, k_fit, T0_fit, T_fit)

plt.figure(figsize=(10, 6))
plt.plot(time, normalized_temperature, label='Normalized Data', color='blue')
plt.plot(time, fitted_temperature, label='Fitted Curve', color='red', linestyle='--')
plt.xlabel('Time (s)')
plt.ylabel('Normalized Temperature (°C)')
plt.title('Curve Fitting: Normalized Temperature vs. Time')
plt.legend()
plt.grid(True)
plt.show()

print(f"Fitted Parameters: k = {k_fit}, T0 = {T0_fit}, T = {T_fit}")
