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

#define PIN_CLOCK_IN 0 // B
#define PIN_LATCH_IN 1 // B
#define PIN_SERIAL_IN 4 // F

#define PIN_CLOCK_OUT 0 // F
#define PIN_LATCH_OUT 1 // F
#define PIN_SERIAL_OUT 2 // B

volatile unsigned char lastClockVal = 3, lastLatchVal = 3;

volatile unsigned short inputs = 0b1111111111111111;

volatile unsigned short keyMask = 0;

volatile unsigned char waitingForClock = 1;

void setupPins(void);
void printBBits(void);
void getInputs(void);

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
		_delay_us(1000);
		getInputs();
		usb_gamepad_send();
	}
}

unsigned char getInput(void) {
	unsigned char ret = (PINF & (1<<PIN_SERIAL_IN));
	_delay_us(6);
	PORTF |= (1<<PIN_CLOCK_OUT);
	_delay_us(6);
	PORTF &= ~(1<<PIN_CLOCK_OUT);
	return ret;
}

void getInputs(void) {
	// pulse latch
	PORTF |= (1<<PIN_LATCH_OUT);
	_delay_us(12);
	PORTF &= ~(1<<PIN_LATCH_OUT);

	unsigned char val;

	for (unsigned char i = 0; i < 16; i++) {
		val = getInput();
		if (val != 0)
			inputs |= (1<<i);
		else
			inputs &= ~(1<<i);

		switch (i) {
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

unsigned char lightV = 0;

ISR(PCINT0_vect) {
	if (waitingForClock == 1) {
		unsigned char clockVal = (PINB & (1 << PIN_CLOCK_IN));
		if (clockVal != lastClockVal) {
			lastClockVal = clockVal;

			if (lastClockVal != 0) {
				if (keyMask != 0) {
					if (keyMask < 0b0001000000000000 && (inputs & keyMask) == 0) {
						PORTB &= ~(1 << PIN_SERIAL_OUT);
					} else {
						PORTB |= (1 << PIN_SERIAL_OUT);
					}
					keyMask = keyMask << 1;
				} else {
					PORTB |= (1 << PIN_SERIAL_OUT);
					waitingForClock = 0;
				}
			}
		}
	}

	unsigned char latchVal = (PINB & (1<<PIN_LATCH_IN));
	if (latchVal != lastLatchVal) {
		lastLatchVal = latchVal;

		if (lastLatchVal != 0) {
			if (lightV > 4)
				LED_OFF;
			else
				LED_ON;

			lightV++;
			if (lightV == 10)
				lightV = 0;
			keyMask = 1;
			waitingForClock = 1;
			if (keyMask != 0) {
				if ((inputs & keyMask) == 0) {
					PORTB &= ~(1 << PIN_SERIAL_OUT);
				} else {
					PORTB |= (1 << PIN_SERIAL_OUT);
				}
				keyMask = keyMask << 1;
			}
			// Already have the inputs, do nothing else
		}
	}
};

void setupPins(void) {
	DDRB &= ~(1<<PIN_CLOCK_IN);
	DDRB &= ~(1<<PIN_LATCH_IN);
	DDRB |= (1<<PIN_SERIAL_OUT);

	DDRF |= 1<<PIN_CLOCK_OUT;
	DDRF |= 1<<PIN_LATCH_OUT;
	DDRF &= ~(1<<PIN_SERIAL_OUT);

	PCICR |= (1 << PCIE0);
	PCMSK0 |= (1<<PIN_CLOCK_IN);
	PCMSK0 |= (1<<PIN_LATCH_IN);
}