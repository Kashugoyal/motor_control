# Motor Control Project - Position control of a Brushed DC motor

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
The menu for the list of commands is given below.
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

### 3. Working 

A basic flowchart for the operation is given below. 
<img src="https://github.com/Kashugoyal/motor_control/blob/master/images/loop.png?raw=true" width=70% align="center" >

* The PWM signal frequency is 20 KHz. This is generated by `Timer3` on the pin `OC1`. 
* The inner loop controls the current through the motor taking feedback from the current sensor. The loop runs at a frequency of 5 KHz which is controlled by the `Timer2`. 
* The outer position control lop runs at a frequncy of 200 Hz controlled by the `Timer4`. The feedback is given by the additional `PIC16` over `SPI` channel which read the encoder and sends the angle value to the `PIC32`. 
* `Matlab` acts as the trajectory generator and also the scope which displays the final result after the trajectory has been executed.

### 4. Charts
* __Current test__
<img src="https://github.com/Kashugoyal/motor_control/blob/master/images/final.jpg?raw=true" width=70% align="center" >
* __Step Trajectory__
<img src="https://github.com/Kashugoyal/motor_control/blob/master/images/track_step.jpg?raw=true" width=70% align="center" >
* __Cubic Trajetory__
<img src="https://github.com/Kashugoyal/motor_control/blob/master/images/final_cubi.jpg?raw=true" width=70% align="center" >
   > Blue line is the plot for desired values while orange is the obtained set of values. 

### 5. References
1. __[ME333 Course Website][1]__
2. __[NU32 development board and book][2]__
3. __[NU32 Software][3]__

[1]: http://hades.mech.northwestern.edu/index.php/ME_333_Introduction_to_Mechatronics
[3]: http://hades.mech.northwestern.edu/index.php/NU32_Software
[2]: http://hades.mech.northwestern.edu/index.php/NU32
