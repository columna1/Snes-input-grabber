# Snes Input Display
An input display made with a teensy 2.0
This input display takes input from a snes controller and outputs it to a console and the computer through usb as an HID gamepad
There is a [companion program](https://github.com/columna1/Love2d-SNES-Input-Display/) made in love2d to visualize the input for use on livestreams and videos
# Schematic
<<<<<<< HEAD
![alt tag](https://raw.github.com/columna1/Snes-input-display/master/schematic-v1.1.png)
=======
![alt tag](https://raw.github.com/columna1/Snes-input-display/master/schematic-v1.0.png)
>>>>>>> 3ea0d45f251f7374fd0786011fb0599dc5888353
# Compiling
[Look at Teensy's website](https://www.pjrc.com/teensy/first_use.html)
#Credits
Thanks to Josh Kropf for the usb gamepad code
Most of the other code made by [Altenius](https://github.com/Altenius)
<<<<<<< HEAD
Guidance, Planning, Testing, Building, and reference from [columna1](https://github.com/columna1)

#changelog
v1.1 Changed design and code to let the console poll the controller directly and then poll the controller in between frames.

v1.0
inital design/code
(did not work
=======

Guidance, Planning, Testing, Building, and reference from [columna1](https://github.com/columna1)

-Note, If you want to use the teensy as a simple snes-usb adapter you can do so if you solder the 5v from the controler to vcc pin of the teensy. This will supply power from the teensy to the snes controller if you want to use it while disconnected from a SNES console. If you are [clever](https://www.pjrc.com/teensy/external_power.html) about it you could even wire power from the SNES to the teensy so that you could use this while disconnected from the computer if you so desired.
>>>>>>> 3ea0d45f251f7374fd0786011fb0599dc5888353
