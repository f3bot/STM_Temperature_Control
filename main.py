from tkinter import *
from tkinter import messagebox
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import serial
import re

class guiApp:           
    def __init__(self):
        self.serial_port = serial.Serial("COM3", 115200, timeout=0.1)
        self.TempData = 0
        self.currentTime = 0
        self.PWMData = 0
        self.setTemp = 0

    def createWindow(self):
        root = Tk()
        root.title("UART Temperature Control")
        root.geometry("800x600")
        return root

    def receive_data(self):        
        data = self.serial_port.readline()
        if data:
            data = data.decode().strip()
            print(data)
            match = re.search(
                r"Timestamp: (\d+) ms, Set Temperature: ([\d.]+)C, Current Temperature: ([\d.]+)C, PWM Value: (\d+)", 
                data
            )
            if match:
                self.currentTime = float(match.group(1)) / 1000
                self.setTemp = float(match.group(2))
                self.TempData = float(match.group(3))
                self.PWMData = int(match.group(4))
                print(self.currentTime, self.setTemp, self.TempData, self.PWMData)
        return [self.currentTime, self.TempData, self.PWMData]

    def runMainLoop(self):
        rootWindow = self.createWindow()
        self.addTempEntry(rootWindow)
        self.addInfoLabels(rootWindow)
        self.createAnimatedPlot(rootWindow)

        def update():
            self.currentTime, self.TempData, self.PWMData = self.receive_data()
            self.updateInfoLabels()
            rootWindow.after(100, update)

        update()
        rootWindow.mainloop()

    def addTempEntry(self, root):
        txt = Entry(root, width=20)
        txt.grid(column=1, row=0, padx=10, pady=10)
       
        submit_button = Button(root, text="Submit temperature", 
                               command=lambda: self.tempEntryCallback(txt.get()))
        submit_button.grid(column=2, row=0, padx=10, pady=10)
     
    def tempEntryCallback(self, tempEntry):
        if not self.validateDataEntry(tempEntry):
            messagebox.showerror("Invalid Input", "Set temperature must be 23 < temp < 50")
        else:
            command = f"SETTEMP{tempEntry}"
            self.serial_port.write(command.encode())
            
            messagebox.showinfo("Temperature Set", f"New temperature set to {tempEntry}")
        
    def validateDataEntry(self, data):
        if data.isdigit():
            temp = int(data)
            return 23 < temp < 50
        return False

    def addInfoLabels(self, root):
        self.setTempLabel = Label(root, text="Set Temperature: -- °C", font=("Arial", 12))
        self.setTempLabel.grid(column=0, row=1, padx=10, pady=10, sticky="W")
        
        self.currentTempLabel = Label(root, text="Current Temperature: -- °C", font=("Arial", 12))
        self.currentTempLabel.grid(column=0, row=2, padx=10, pady=10, sticky="W")
        
        self.pwmLabel = Label(root, text="PWM Value: --", font=("Arial", 12))
        self.pwmLabel.grid(column=0, row=3, padx=10, pady=10, sticky="W")

    def updateInfoLabels(self):
        self.setTempLabel.config(text=f"Set Temperature: {self.setTemp:.2f} °C")
        self.currentTempLabel.config(text=f"Current Temperature: {self.TempData:.2f} °C")
        self.pwmLabel.config(text=f"PWM Value: {self.PWMData}")

    def createAnimatedPlot(self, root):
        style.use('fivethirtyeight')
        fig, ax = plt.subplots(figsize=(6, 4))
        ax.set_title("Live Temperature Plot")
        ax.set_xlabel("Time (s)")
        ax.set_ylabel("Temperature (°C)")
        
        xs = []
        ys = []
        
        def animate(i):
            if len(xs) > 100:
                xs.pop(0)
                ys.pop(0)
            xs.append(self.currentTime)
            ys.append(self.TempData)
            ax.clear()
            ax.plot(xs, ys, label="Temperature")
            ax.set_title("Live Temperature Plot")
            ax.set_xlabel("Time (s)")
            ax.set_ylabel("Temperature (°C)")
            ax.legend(loc="upper right")
        
        canvas = FigureCanvasTkAgg(fig, master=root)
        canvas_widget = canvas.get_tk_widget()
        canvas_widget.grid(column=0, row=4, columnspan=3, padx=10, pady=10)
        
        ani = animation.FuncAnimation(fig, animate, interval=1000)  
        canvas.draw()

gui = guiApp()
gui.runMainLoop()
