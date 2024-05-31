
/*
 * isr.c
 *
 * Created: 20.05.2024
 *  Author: Elektronaut
 */

#include "includes.h"

ISR(PCINT0_vect) {
	// Check if the interrupt was caused by PB2 going low
	if (!(INPUTS_PORT & (1 << PIN_FEEDBACK))) {
		g_shutter_opening_timeout = 0; //disable timeout!		
		PCMSK0 &= ~(1 << PCINT2); // Disable pin change interrupt on PCINT2
		if (g_flash_enabled) {
			OUTPUT_PORT |= (1 << FLASH_OUT_PIN);
			g_tick_flash_duration = 11;
			g_flash_has_fired = true;
		}
	}
}

/* Inputs are read, statemachine indepdent and state machine are run here
is run @ 1 kHz.
checking finshied would be an option if 
*/
ISR(STATEMACHINE_TIMER_ISR) {
	// if (!finished)  { error }
	// finished = false;

	PORTD ^= (1 << PD2); // Debug feature to measure execution time with an oscilloscope on PD2
	Read_Inputs();
	State_Independent();
	Statemachine();
	PORTD ^= (1 << PD2);
}