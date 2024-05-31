/*
 * modules.c
 *
 * Created: 20.05.2024 01:30:22
 *  Author: Elektronaut
 */
#include "includes.h"

#ifdef STATEMACHINE_USE_TIMER_2
void setup_timer2(void) {
	TCCR2A = (1 << WGM21); //CTC Mode, (Page 197)
	TCCR2B |= (1 << CS22); // Set prescaler to 64 (Page 198)
	OCR2A = 249; // Für a 1 kHz Tick:  Fclk / (Prescaler * ftick) -1 || 16000000 / (64 * 1000) -1 = 249
	TIMSK2 = (1 << OCIE2A); //OCI Int active
}
#endif

#ifdef STATEMACHINE_USE_TIMER_1
void setup_timer1(void) {
	TCCR1B |= (1 << WGM12); //CTC (Seite 154)
	TCCR1B |= (1 << CS10) | (1 << CS11); //Prescaler auf 64 (Seite 155)
	OCR1A = 249;  //1 kHz Takt
	TIMSK1 |= (1 << OCIE1A); //OCI Int aktiviern
}
#endif