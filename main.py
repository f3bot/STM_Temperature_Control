from tkinter import *
from tkinter import messagebox
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import matplotlib.pyplot as plt
import matplotlib.animation as animation
from matplotlib import style
import random  # For simulating live data

class guiApp:
    def createWindow(self):
        root = Tk()
        root.title("UART Temperature Control")
        root.geometry("800x600")
        return root
        
    def runMainLoop(self):
        rootWindow = self.createWindow()
        self.addTempEntry(rootWindow)
        self.createAnimatedPlot(rootWindow)
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
            messagebox.showinfo("Temperature Set", f"New temperature set to {tempEntry}")
        
    def validateDataEntry(self, data):
        if data.isdigit():
            temp = int(data)
            return 23 < temp < 50
        return False
        
    def createAnimatedPlot(self, root):
        style.use('fivethirtyeight')
        fig, ax = plt.subplots(figsize=(6, 4))
        ax.set_title("Live Temperature Plot")
        ax.set_xlabel("Time")
        ax.set_ylabel("Temperature")
        
        xs = []
        ys = []
        
        def animate(i):
            xs.append(len(xs) + 1)  # Increment x values
            ys.append(random.uniform(20, 50))  # Simulate temperature readings
            ax.clear()
            ax.plot(xs, ys, label="Temperature")
            ax.set_title("Live Temperature Plot")
            ax.set_xlabel("Time")
            ax.set_ylabel("Temperature")
            ax.legend(loc="upper right")
        
        canvas = FigureCanvasTkAgg(fig, master=root)
        canvas_widget = canvas.get_tk_widget()
        canvas_widget.grid(column=0, row=1, columnspan=3, padx=10, pady=10)
        
        ani = animation.FuncAnimation(fig, animate, interval=1000)  # Update every second
        canvas.draw()

gui = guiApp()
gui.runMainLoop()
