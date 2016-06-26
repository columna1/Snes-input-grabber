//version 1.1 snescontrol.c by Altenius and columna1

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>

#include "usb_gamepad.h"

#if defined(__AVR_ATmega32U4__) || defined(__AVR_AT90USB1286__)
#define LED_ON		(PORTD |= (1<<6))
#define LED_OFF		(PORTD &= ~(1<<6))
#endif

#define PINB_FALL(pin) while(PINB & (1 << pin));
#define PINB_HIGH(pin) while(!(PINB & (1 << pin)));
#define PINB_HIGHFALL(pin) PINB_HIGH(pin) PINB_FALL(pin)

#define LED_CONFIG	(DDRD |= (1<<6))
#define CPU_PRESCALE(n)	(CLKPR = 0x80, CLKPR = (n))

#define PIN_SERIAL 2 // B2 : Serial data coming from controller

#define PIN_CLOCK 0 // B0 : Data clock going to controller
#define PIN_LATCH 1 // B1 : Data latch going to controller

void setupPins(void);
static void updateGamepadState(void);

static inline unsigned char getInput(void) {
	_delay_us(6);
	PORTB |= (1 << PIN_CLOCK); //set clock line to high
	_delay_us(6);
	PORTB &= ~(1<<PIN_CLOCK); //set clock line to low
	unsigned char ret = (PINB & (1<<PIN_SERIAL));
	return ret;
}

static inline void pulseLatch(void) {
	PORTB |= (1<<PIN_LATCH);
	_delay_us(12);
	PORTB &= ~(1<<PIN_LATCH);
}

int main(void) {
	CPU_PRESCALE(0);
	LED_CONFIG;

	usb_init();
	while (!usb_configured()); // wait

	_delay_ms(1000);
	usb_gamepad_reset_state();

	cli();
	setupPins();
	sei();

	unsigned char val;

	while(1)
	{

		PINB_HIGHFALL(PIN_LATCH);
		val = (PINB & (1<<PIN_SERIAL));
		gamepad_state.b_btn = (val == 0);
		PINB_FALL(PIN_CLOCK);

		for (int i = 1; i < 16; i++) {
			PINB_HIGHFALL(PIN_CLOCK);
			val = (PINB & (1<<PIN_SERIAL));

			switch (i) {
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

		usb_gamepad_send();
	}
}

void setupPins(void) {
	DDRB &= ~(1<<PIN_SERIAL); // Sets serial data to input

	DDRB &= ~(1<<PIN_CLOCK); // Sets data clock to input
	DDRB &= ~(1<<PIN_LATCH); // Sets data latch input
}
