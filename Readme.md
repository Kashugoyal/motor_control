
# Motor Control Project - Position control of a Brushed DC motor
***
__This package demonstrates the control of a brushed DC motor with encoder feedback. We use a PIC32 microcontroller (on an NU32 board) for the position control and the interface is developed in Matlab as a menu i.e. a list of commands to choose from and send instructions to the PIC.__

### 1. File Info
* __control.c__
   This is the file that contains the `main()` function. The various interrupt sequences as well as timer initiations are defined in this file. This file also contains the menu commands for receiving MAtlab commands.
* __encoder.c__
   This file contains the definitions for the encoder functions.
* __isense.c__
   This file contians function definitions for the Ananlog to Digital Convertor Pin which reads the current values for the inner feeedback loop.
* __NU32.c__
   This file is the library of useful functions for the NU32 board.
* __utilities.h__
   This file contatins the helper functions for the control program.
* __client.m__
   This file is run in Matlab. Using serial communication the code sends these commands to the PIC. The code also reads the plot points and calls the functions to plot the desired and followed trajectories.
* __read_plot_matrix.m__
   This file is used to generate the plot of the points received over the `UART`. The desired and the actual trajectories are shown in the same plot.
* __genRef.m__
   For the menu items `m` and `n` this file contains the function to generate the data points for the step and cubic trajectories respectively.   


### 2. Menu
The menu for the list of commands is goven below.
```
a: Read current sensor (counts) 
b: Read current senson (mA) 
c: Read encoder (counts)        
d: Read encoder (deg)
e: reset encoder                
f: Set PWM (-100 to 100) 
g: Set Current Gains            
h: Get Current Gains
i: Set Position Gains
j: Get Position Gains
k: Test Current Control
l: Go to angle (deg)
m: Load Step Trajectory
n: Load Cubic Trajectory
o: Execute Trajectory
p: Unpower the motor
r: Get mode
q: Quit
```

