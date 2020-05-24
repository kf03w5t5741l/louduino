# Louduino

Makes an LED light up according to sound level measured by a microphone. Sound level is measured by collecting a series of samples and calculating the root mean square (rms). LED brightness is simulated via pulse-width modulation (PWM). A potentiometer adjusts input sensitivity.

![](louduino_demonstration.gif)

## Technical specs

* Board:  Arduino UNO Rev 3.0 (ATMega238P)
* Input:  Electret MAX4466 mic+amp (VCC: 3.3V, GND, OUT: pin A_0)
* Output: MOSFET IRF520 via pin 11 (PWM), switching the LED circuit
* Analog reference: 3.3V via Arduino UNO 3.3V pin 
