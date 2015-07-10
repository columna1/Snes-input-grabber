#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usb_gamepad.h"

#if defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB1286__)
#define LED_ON		(PORTD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))
#endif

#define LED_CONFIG	(DDRD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

#define PIN_CLOCK_IN 0 // B0 : Data clock coming from console
#define PIN_LATCH_IN 1 // B1 : Data latch coming from console
#define PIN_SERIAL_IN 4 // F4 : Serial data going to console

#define PIN_CLOCK_OUT 0 // F0 : Data clock going to controller
#define PIN_LATCH_OUT 1 // F1 : Data latch going to controller
#define PIN_SERIAL_OUT 2 // B2 : Serial data coming from controller

volatile unsigned char lastClockVal = 3, lastLatchVal = 3;

volatile unsigned short inputs = 0b1111111111111111;

volatile unsigned short keyMask = 0;

volatile unsigned char waitingForClock = 0;

void setupPins(void);//Set the pins input/output as needed
void printBBits(void);
void getInputs(void);//Get the state of the controller

int main(void) {
	//set the clock to the highest available
	CPU_PRESCALE(0);
	LED_CONFIG;
	LED_OFF;

	//init usb communication for HID device section (to act like a joystick)
	usb_init();
	while (!usb_configured()); // wait until it is configured

	_delay_ms(1000);//wait a bit to be safe 
	usb_gamepad_reset_state();

	cli();//stuff for interupts
	setupPins();//sets the pins up
	sei();

	while(1)//loop and get/send the controller's state
	{
		_delay_us(12);
		getInputs();
		usb_gamepad_send();
	}
}


unsigned char getInput(void) {
	//polls the controller for the states of one the buttons
	unsigned char ret = (PINF & (1<<PIN_SERIAL_IN));
	_delay_us(6);
	PORTF |= (1<<PIN_CLOCK_OUT);
	_delay_us(6);
	PORTF &= ~(1<<PIN_CLOCK_OUT);
	return ret;
}

void getInputs(void) {
	// pulse latch, tell the controller we want its buttons
	PORTF |= (1<<PIN_LATCH_OUT);
	_delay_us(12);
	PORTF &= ~(1<<PIN_LATCH_OUT);

	unsigned char val;

	for (unsigned char i = 0; i < 16; i++) {//for every button go through and get it/send it to the comptuer
		val = getInput();
		if (val != 0)//put the state into the inputs variable to use for sending to the console
			inputs |= (1 << i);
		else
			inputs &= ~(1 << i);

		switch (i) {//set the variables to send to computer
			case 0: // B
				gamepad_state.b_btn = (val == 0) ? 1 : 0;
				break;
			case 1: // Y
				gamepad_state.y_btn = (val == 0) ? 1 : 0;
				break;
			case 2: // Select
				gamepad_state.select_btn = (val == 0) ? 1 : 0;
				break;
			case 3: // Start
				gamepad_state.start_btn = (val == 0) ? 1 : 0;
				break;
			case 4: // up
				if (val == 0)
					gamepad_state.y_axis = 0;
				else
					gamepad_state.y_axis = 0x80;
				break;
			case 5: // Down
				if (val == 0) {
					if (gamepad_state.y_axis == 0)
						gamepad_state.y_axis = 0x80;
					else
						gamepad_state.y_axis = 0xFF;
				}
				break;
			case 6: // Left
				if (val == 0)
					gamepad_state.x_axis = 0;
				else
					gamepad_state.x_axis = 0x80;
				break;
			case 7: // Right
				if (val == 0) {
					if (gamepad_state.x_axis == 0)
						gamepad_state.x_axis = 0x80;
					else
						gamepad_state.x_axis = 0xFF;
				}
				break;
			case 8: // A
				gamepad_state.a_btn = (val == 0) ? 1 : 0;
				break;
			case 9: // X
				gamepad_state.x_btn = (val == 0) ? 1 : 0;
				break;
			case 10: // L
				gamepad_state.l_btn = (val == 0) ? 1 : 0;
				break;
			case 11: // R
				gamepad_state.r_btn = (val == 0) ? 1 : 0;
				break;
		}
	}
}

volatile unsigned char clockVal, latchVal;

ISR(PCINT0_vect) {//Interupt routine
	if (waitingForClock) {//if we are currently listening for clock
		clockVal = (PINB & (1 << PIN_CLOCK_IN));//get the state of the clock to see if it is a rising/falling edge
		if (clockVal != lastClockVal) { //if it is a different state than last time aka this interupt is for us
			lastClockVal = clockVal;//set this for later

			if (lastClockVal != 0) {//if it is the rising edge
				if (keyMask != 0) {//and we are sending the controller state
					if ((inputs & keyMask) == 0) { // Check if the button on the controller is depressed and set the output for the console to read
						PORTB &= ~(1 << PIN_SERIAL_OUT); // depressed
					} else {
						PORTB |= (1 << PIN_SERIAL_OUT); // raised
					}
					keyMask = keyMask << 1;//shift the mask to be used for next cycle
				} else {//we aren't sending the controlls/we are done sending them
					PORTB &= ~(1 << PIN_SERIAL_OUT);//set to 0 and wait for next latch
					waitingForClock = 0;
				}
			}
		}
	}

	latchVal = (PINB & (1<<PIN_LATCH_IN));//get the state of the latch to see if it changed
	if (latchVal != lastLatchVal) {//if it changed
		lastLatchVal = latchVal;

		if (lastLatchVal != 0) {//if this is the rising edge
			keyMask = 1;//reset the key mask
			waitingForClock = 1;//let us know we are sending to the controller
			//send out the first button
			if (keyMask != 0) {
				if ((inputs & keyMask) == 0) { // Check if the button is depressed
					PORTB &= ~(1 << PIN_SERIAL_OUT); // depressed
				} else {
					PORTB |= (1 << PIN_SERIAL_OUT); // raised
				}
				keyMask = keyMask << 1;
			} else {
				PORTB &= ~(1 << PIN_SERIAL_OUT);
				waitingForClock = 0;
			}
			// Already have the inputs, do nothing else
		}
	}
};

void setupPins(void) {
	DDRB &= ~(1<<PIN_CLOCK_IN); // Sets data clock coming from console to input
	DDRB &= ~(1<<PIN_LATCH_IN); // Sets data latch coming from console to input
	DDRB |= (1<<PIN_SERIAL_OUT); // Sets serial data going to console to output

	DDRF |= 1<<PIN_CLOCK_OUT; // Sets data clock going to controller to output
	DDRF |= 1<<PIN_LATCH_OUT; // Sets data latch going to controller to output
	DDRF &= ~(1<<PIN_SERIAL_OUT); // Sets serial data coming from controller to input

	PCICR |= (1 << PCIE0); // Enables the PCINT0 interrupt
	PCMSK0 |= (1<<PIN_CLOCK_IN); // Enables the interrupt for data clock
	PCMSK0 |= (1<<PIN_LATCH_IN); // Enables interrupt for data latch
}
