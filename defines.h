/*
 * defines.h
 *
 * Created: 20.05.2024
 *  Author: Elektronaut
 */ 


#ifndef DEFINES_H_
#define DEFINES_H_



/* inputs*/
#define INPUTS_PORT PINB
#define PIN_PREVIEW PB1 
#define PIN_RELEASE PB0 
#define PIN_FEEDBACK PB2 
#define PCINT_PIN PCINT2 // muss den gelichen pin wie PIN_FEEDBACK beschreiben


/* Define inputs */
//analog input number
#define PIN_DREHWAHLSCHALTER 0

#define OUTPUT_PORT PORTD
#define FLASH_OUT_PIN PD7

//#define SHUTTER_OUT_PIN PD6  //pd6 kann nicht verwendet werden, da COM0A1 = 1 mit WGM02 = 1 nicht zusammenspielt! Siehe Seite 125, A special case...
#define SHUTTER_OUT_PIN PD5
#define SHUTTER_LED_OUT PD4

#define numPos 12  //Number of positions of rotary contact switch

/* Macros for input pin states */
#define BTN_PREVIEW (g_buttons & (1<< PIN_PREVIEW ))
#define BTN_RELEASE (g_buttons & (1<< PIN_RELEASE ))
#define BTN_FEEDBACK (g_buttons & (1 << PIN_FEEDBACK))


/* stuff for PWM generation*/
#define PWM_TICK 251  //wartezeit in ms für PWM aktivierung
#define PWM40_A 79   //Parameter für 25 kHz 40% PWM
#define PWM40_B 31

/* delays in ticks */
#define TIMEOUT_SHUTTER 1000
#define TIMEOUT_CLOSING 250
#define DEBOUNCE_DELAY_MS 15
#define DELAY_INIT 150
#define ERROR_BLINK 333

/* Lookup table for shutter times*/
#define TIME_T1_60 16
#define TIME_T1_30 33
#define TIME_T1_15 66
#define TIME_T1_8 125
#define TIME_T1_4 250
#define TIME_T1_2 500
#define TIME_T1 1000
#define TIME_T2 2000
#define TIME_T5 5000
#define TIME_T10 10000
#define TIME_MIN 70  //unteres limit für vershclussöffnung
#define FAST_SHUTTER_MARKUP 40
#define SLOW_SHUTTER_MARKUP 20

/* Define which time to use for state machine clock */
#define STATEMACHINE_USE_TIMER_2

#ifdef STATEMACHINE_USE_TIMER_1
#define STATEMACHINE_TIMER_SETUP setup_timer1
#define STATEMACHINE_TIMER_ISR TIMER1_COMPA_vect
#endif

#ifdef STATEMACHINE_USE_TIMER_2
#define STATEMACHINE_TIMER_SETUP setup_timer2
#define STATEMACHINE_TIMER_ISR TIMER2_COMPA_vect
#endif

#endif
