
/*
 * main.c
 *
 * Created: 11.05.2024
 *  Author: Elektronaut
 */ 


#include "includes.h"
int main(void);

int main(void) {
	Setup_System();
	while(1) {;}
	return -1;
}

/* Read buttons and debounce inputs by comparing button state with the previous. If a change in buttons state is detected the changed state is written to the global variabl g_buttons and furhter reads a prevented for DEBOUNCE_DELAY_MS ms this is to allow the  the contacts to settle. This implementation allows to reacly quickly (withon one system tick) to a change in button state but still prevents bouncing.
*/
void Read_Inputs(void) {
	static uint8_t temp_buttons;

	if (g_tick_debounce == 0) {
		temp_buttons = (~INPUTS_PORT) & 0x07; // 0000.0111 maske
		if (g_buttons ^ temp_buttons) {
			g_tick_debounce = DEBOUNCE_DELAY_MS;	
			g_buttons = temp_buttons;
		}
	}
}

/* Read ADC channel, blocks until read is finished
is fast enought to not interfere with system tick
*/
uint16_t Read_ADC(uint8_t channel) {
	// select MUX and channel
	ADMUX = (ADMUX & 0xF8) | (channel & 0x07);    
	// start ADC
	ADCSRA |= (1 << ADEN) | (1 << ADSC);    
	// now elevator music while we wait
	while (ADCSRA & (1 << ADSC)) ;    
	// Return ADC val
	return ADC;
}

/* Einrichten I/O und Timer für Statemachine aktivieren
Interrupts aktivieren
	System setup:
	* Setup outputs (all on PORTD)
	* Setup inputs (all on PORTB)
	* Setup ADC 
	* set inital state of state machine: State_init
	* Initialize System tick 
*/
void Setup_System(void) {
	/* Outputs festlegen: 
	PD2 Dbug Task load
	PD7 PC/flash trigger
	PD6 Power/ready indicator LED
	PD5 Output to N-Channel FET to drive the solenoid of the shutter
	PD4 Shutter staus indicator, shows if the shutter is currentlly being driven (on or PWM)
	PD3 not used right now
	xxx On the arduino UNO dev baord PD0 & PD1  are used for something else
	*/
	DDRD |= (1 << PD7) |(1 << PD6) | (1 << PD5) | (1 << PD4) | (1 << PD3) | (1<<PD2);
	PORTD = 0x00;

	/* Setup Inputs
	Set inputs for Shutter release, focus switch; shutter-open-feedback
	and enable pulls-up resistors by written a 1 to the output bits
	*/
	DDRB &= ~(1 << PIN_RELEASE) & ~(1<<PIN_PREVIEW) & ~(1<<PIN_FEEDBACK);  
    PORTB = (1<<PIN_RELEASE) | (1<<PIN_PREVIEW) | (1<<PIN_FEEDBACK); 

	/* setup ADC
	*/
	ADMUX |= (1 << REFS0);    //Aref = Vcc
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // 128 prescaler	

	state = State_Initialize;  // Set to State_Initialize, that's done to await possible power glitches.

	STATEMACHINE_TIMER_SETUP();  // We use Timer2, that one has a higher priority and is able to do an exact 1 kHz tick
	sei(); // enable interrupts... and start the show
	
}

/* Setup and enable fast-PWM in noninverting mode on PD5
This takes control over PD5 from the PORTD register
*/
void Enable_PWM(void) { 
	TCCR0A |= (1 << WGM01) | (1 << WGM00) | ( 1 << COM0B1) ;  //PD5!!
	TCCR0B |= (1 << CS01) | (1<<WGM02);    
	// Initial duty cycle: 40 % @ 25 kHZ 79/31
	OCR0A = PWM40_A;  //79
	OCR0B = PWM40_B;  //31
}

/* Disable PWM and hand back control over PD5 to PORTD register
*/
void Disable_PWM(void) {
	TCCR0B &= ~(1 << CS01);  // Stop timer
	TCNT0 = 0; // reset timer
	TCCR0A &= ~( 1 << COM0B1); // Give control over SHUTTER_OUT_PIN back to PORTD register
}

/* Read and decode rotary selector switch 
*/
void Read_rotary_switch(void) {
	uint16_t adcValue = Read_ADC(PIN_DREHWAHLSCHALTER);
	g_mode_switch = adcValue * numPos / 1024 + 1; 	//Ausrechnen von Position
}

/* Enable pin change interrupt on PB2 to:
a) confirm the shutter has opened
b) trigger flash when needed
*/
void Enable_PC_PCINT(void) {    
	PCMSK0 |= (1 << PCINT_PIN); // Enable PCINT2 Pin 0 aka. PB2
	PCICR |= (1 << PCIE0); // Enable intterrupt on PCIE0 
}

/* Disable pin change interrupt
*/
void Disable_PC_PCINT(void) {
	PCICR &= ~(1 << PCIE0); //Disable PCIE0
	PCMSK0 &= ~(1 << PCINT_PIN); // Disable PCINT0 (PB2)
}



/* Open shutter, enable flash */
void Shutter_open_with_flash_with_pwm(void) {
	Shutter_Open();
	g_flash_enabled = true;
	Enable_PC_PCINT();
	g_tick_pwm = PWM_TICK; 
}


/* Open shutter, ste PWM-tick, disable flash, enable shutter feedback pc interrupt
is used for opening the shutter without firing the flash, is used for focusing 
*/
void Shutter_open_with_pwm(void) {
	Shutter_Open();
	g_tick_pwm = PWM_TICK;
	g_flash_enabled = false;
	Enable_PC_PCINT();
}

/* Open shutter, enable shutter enabled inducator LED, set g_shutterWirdAngesteuert flag, reset g_flash_has_fired flag and wind up timer for timeout!
*/
void Shutter_Open(void) {
	OUTPUT_PORT |= (1 << SHUTTER_OUT_PIN); // An alternative would be to enable PWM % 100 %
	OUTPUT_PORT |= (1 << SHUTTER_LED_OUT);  // Enable shutter enabled indicator LED
	
	g_shutter_is_activated = true;
	g_flash_has_fired = false;
	g_shutter_opening_timeout = TIMEOUT_SHUTTER;
}

/* Close shutter, disable g_shutterWirdAngesteuert flag and disable timeout */
void Shutter_Close(void) {
	g_shutter_is_activated = false;
	Disable_PWM();  // Regain control over I/O pin from Timer 0 PWM unit...
	OUTPUT_PORT &= ~(1 << SHUTTER_OUT_PIN); // ... and then closing shutter. An alternative would be to set PWM to 0 % or disable it 
	OUTPUT_PORT &= ~(1 << SHUTTER_LED_OUT); // Disable shutter enabled indicator LED
	g_shutter_opening_timeout = 0; // Disable timeout	
}

/* Lookup-Table für shutter times 
*/
void Shutter_OpenAndStartTimer(void) {
	uint16_t ms;
	switch (g_mode_switch)	{
		case t1_60:
			ms = TIME_T1_60;
			break;
		case t1_30:
			ms = TIME_T1_30;
			break;
		case t1_15:
			ms = TIME_T1_15;
			break;
		case t1_8:
			ms = TIME_T1_8;
			break;
		case t1_4:
			ms = TIME_T1_4;
			break;
		case t1_2:
			ms = TIME_T1_2;
			break;
		case t1:
			ms = TIME_T1;
			break;
		case t2:
			ms = TIME_T2;
			break;
		case t5:
			ms = TIME_T5;
			break;
		case t10:
			ms = TIME_T10;
			break;
		default:
			ms = 0;
			break;
	}
	if ( ms < TIME_MIN) {
		ms += FAST_SHUTTER_MARKUP;
	} else {
		ms += SLOW_SHUTTER_MARKUP;
	}
	
	g_tick_shutter = ms;
	Shutter_open_with_flash_with_pwm();
}
