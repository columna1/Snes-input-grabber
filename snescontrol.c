//version 1.1 snescontrol.c by alakazard and columna1

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

// #define PIN_CLOCK_IN 1 << 0 // B0 : Data clock coming from console
// #define PIN_LATCH_IN 1 << 1 // B1 : Data latch coming from console
#define PIN_SERIAL 2 // B4 : Serial data coming from controller

#define PIN_CLOCK 0 // B0 : Data clock going to controller
#define PIN_LATCH 1 // B1 : Data latch going to controller
// #define PIN_SERIAL_OUT 2 // B2 : Serial data coming from controller

volatile unsigned char lastLatchVal = 3;

void setupPins(void);
static void getInputs(void);

int main(void) {
	CPU_PRESCALE(0);
	LED_CONFIG;
	LED_OFF;

	usb_init();
	while (!usb_configured()); // wait

	_delay_ms(1000);
	usb_gamepad_reset_state();

	cli();
	setupPins();
	sei();

	while(1)
	{
		//_delay_us(16670);
		//getInputs();
		_delay_ms(1000);
		//usb_gamepad_send();
	}
}

static inline unsigned char getInput(void) {
	_delay_us(6);
	PORTB |= (1<<PIN_CLOCK);//set clock line to high
	_delay_us(6);
	unsigned char ret = (PINB & (1<<PIN_SERIAL));
	PORTB &= ~(1<<PIN_CLOCK);//set clock line to low
	return ret;
}

static inline void getInputs(void) {
	// pulse latch
	PORTB |= (1<<PIN_LATCH);
	_delay_us(12);
	PORTB &= ~(1<<PIN_LATCH);

	_delay_us(6);

	unsigned char val;

	for (unsigned char i = 0; i < 16; i++) {
		val = getInput();
		
		switch (i) {
			case 0: // B
				gamepad_state.b_btn = (val == 0);
				break;
			case 1: // Y
				gamepad_state.y_btn = (val == 0);
				break;
			case 2: // Select
				gamepad_state.select_btn = (val == 0);
				break;
			case 3: // Start
				gamepad_state.start_btn = (val == 0);
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
				gamepad_state.a_btn = (val == 0);
				break;
			case 9: // X
				gamepad_state.x_btn = (val == 0);
				break;
			case 10: // L
				gamepad_state.l_btn = (val == 0);
				break;
			case 11: // R
				gamepad_state.r_btn = (val == 0);
				break;
		}
	}
}

volatile unsigned char latchVal;

volatile unsigned char reading = 0;

static inline void onLatchPulse(void) {

	_delay_ms(2);

	reading = 1;
	getInputs();
	reading = 0;
	usb_gamepad_send();
}

ISR(PCINT0_vect) {
	latchVal = (PINB & (PIN_LATCH));
	if (latchVal != lastLatchVal) {
		lastLatchVal = latchVal;
		if (!reading && latchVal == 0)
			onLatchPulse();
	}
}

void setupPins(void) {
	DDRB &= ~(1<<PIN_SERIAL); // Sets serial data to input

	DDRB |= 1<<PIN_CLOCK; // Sets data clock to output
	DDRB |= 1<<PIN_LATCH; // Sets data latch output

	PCICR |= (1 << PCIE0); // Enables the PCINT0 interrupt
	// PCMSK0 |= (PIN_CLOCK); // Enables the interrupt for data clock
	PCMSK0 |= (PIN_LATCH); // Enables interrupt for data latch
}