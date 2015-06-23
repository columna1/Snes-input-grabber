# Snes Input Display
An input display made with a teensy 2.0
This input display takes input from a snes controller and outputs it to a console and the computer through usb as an HID gamepad
There is a [companion program](https://github.com/columna1/Love2d-SNES-Input-Display/) made in love2d to visualize the input for use on livestreams and videos
# Schematic
![alt tag](https://raw.github.com/columna1/Snes-input-display/master/schematic-v1.0.png)
# Compiling
[Look at Teensy's website](https://www.pjrc.com/teensy/first_use.html)
#Credits
Thanks to Josh Kropf for the usb gamepad code
Most of the other code made by [Altenius](https://github.com/Altenius)

Guidance, Planning, Testing, Building, and reference from [columna1](https://github.com/columna1)

-Note, If you want to use the teensy as a simple snes-usb adapter you can do so if you solder the 5v from the controler to vcc pin of the teensy. This will supply power from the teensy to the snes controller if you want to use it while disconnected from a SNES console. If you are clever you could even wire power from the SNES to the teensy so that you could use this while disconnected from the computer if you so desired.
