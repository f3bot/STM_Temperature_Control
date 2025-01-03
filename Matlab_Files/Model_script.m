% clear all; close all; clc;

% Read and parse data
data = dlmread('proszedzialaj.csv', ',', 2, 0);
t = data(:,1); 
y = data(:,2);
temp_offset = y(1);
y = y - y(1); % normalize data (start from 0)

% Plot step response
plot(t, y, 'r', 'LineWidth', 5);
title("Normalized model data, PMW duty 100%");
xlabel("Time (s)");
ylabel("Temperature (C)");
xlim([0 max(t)]);

% Estimated transfer function
k = 23.27;
T = 290;
T0 = .5;
est_model = tf([k], [T 1], 'InputDelay', T0);
hold on;
step(est_model);
legend("Real step response", "Estimated step response", 'Location', 'southeast');

% PID controller in simulink
sim('UAR.slx')