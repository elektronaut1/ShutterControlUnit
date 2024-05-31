/*
 * statemachine.c
 *
 * Created: 20.05.2024
 *  Author: Elektronaut
 */ 

#include "includes.h"

void Statemachine(void) {
	switch (state) {
		case State_Idle:
		/* Check if any buttons are pressed and change state accordingly. If no buttons are pressed remain in this state
		*/				
			if (BTN_PREVIEW) {
				state = State_Preview; // Open shutter without triggering the flash
			} else if (BTN_RELEASE) {
				Read_rotary_switch(); // say cheese! Find out what mode the dial is set to
				if (g_mode_switch == Mode_Bulb) {
					state = State_Bulb; // Open shutter for as long as the shutter release is pressed down. Flash is triggered.
				} else if (g_mode_switch == Mode_Toggle) {
					state = State_Toggle; // Open shutter with the first press of the shutter release button and and once it's released wait for it to be predded again. Flash is triggerd. 
				} else {
					state = State_Timer; // Open shutter, set timer and close shutter once time has passed. Flash is triggerd. 
				}
			}
			break;
		case State_Preview:
		/* Call function to open shutter (without a timer, and without triggering the flash) and go to state State_Preview_Open.
		*/
			Shutter_open_with_pwm();
			state = State_Preview_Open;
			break;
		case State_Preview_Open:
		/* Wait as long as the focusing button is pressed, then, once it's release go to state State_Shutter_Closing.
		*/
			if (!BTN_PREVIEW) {
				state = State_Shutter_Closing;
			}
			break;		
		case State_Bulb:
		/* Call function to open shutter (without a timer, but with flash active) and go to state State_Bulb_Open.
		*/	
			Shutter_open_with_flash_with_pwm();
			state = State_Bulb_Open;
			break;
		case State_Bulb_Open:
		/* Wait as long as the shutter relase button is pressed, then, once it's release go to state State_Shutter_Closing.
		*/
			if (!BTN_RELEASE) {
				state = State_Shutter_Closing;
			}
			break;
		case State_Toggle:
		/* Call function to open shutter (without a timer, but with flash active) and switch to Sate_Toggle_Btn_Still_Pressed.
		*/
			Shutter_open_with_flash_with_pwm();
			state = State_Toggle_Btn_Pressed;
			break;
		case State_Toggle_Btn_Pressed:
		/* Wait for the shutter relase button to be released ...
		*/
			if (!BTN_RELEASE) {
				state = State_Toggle_Open;
			}
			break;
		case State_Toggle_Open:
		/* ... and then wait for the shutter release to be presse again to switch to State_Shutter_Closing.
		*/
			if (BTN_RELEASE) {
				state = State_Shutter_Closing;
			}
			break;
			
		case State_Timer:
		/* Function to open shutter and set timer is called then the state is swtiched to State_Timer_Open.
		*/
			Shutter_OpenAndStartTimer();
			state = State_Timer_Open;
			break;
		case State_Timer_Open:
		/* Wait for the time to pass and to close the shutter. The the state is switched to State_Shutter_Closing
		*/
			if (g_tick_shutter == 0) { 
				state = State_Shutter_Closing;		
			}
			break;			
		case State_Shutter_Closing:
			//Delay damit der Verschluss schließen kann und nicht sofort wieder eine aufnahme ausgelöst wird
			Shutter_Close();
			g_tick_close_delay = TIMEOUT_CLOSING;
			state = State_Shutter_Closed;			
			break;
		case State_Shutter_Closed:
		/* Wait for the time delay set by State_Shutter_Closing to pass and make sure shutter release and focus are not active to rpevent immediate retriggering of the shutter. Then go to State_Idle
		*/
			if (g_tick_close_delay == 0 && !BTN_RELEASE && !BTN_PREVIEW) {
				Disable_PC_PCINT();
				state = State_Idle;
			}
			break;
		case State_Initialize:
		/* Initial state, set 150 ticks delay and got to state State_Initializing
		*/
			g_tick_init = DELAY_INIT;
			state = State_Initializing;
			break;	
		case State_Initializing:
		/* Wait for delay set by State_Initialize to pass, then activate power indicator LED and go to state State_Idle.
		*/
			if (g_tick_init == 0) {
				state = State_Idle;
				PORTD |= (1<<PD6); //Power LED active
			}
			break;
		case State_Timeout:
		/* This is an error state that gets active when there is not signal from the flash contact in the shutter module. Like when the shutter is stuck or not connected. First the power indicatior LED is disabled and then globals and ticks are reset to their inital state. Close the shutter, disable the pin change interruot and switch to state State_Timeouting. 
		*/
		// Opeing action was a dud
			PORTD &= ~(1<<PD6);
			OUTPUT_PORT &= ~(1 << FLASH_OUT_PIN); // Disable flash output
			g_tick_shutter = 0;
			g_tick_close_delay = 0;
			g_tick_pwm = 0;
			g_flash_has_fired = false;
			Disable_PC_PCINT();
			Shutter_Close();
			g_flash_enabled = false;
			g_error_state_blink = ERROR_BLINK;
			state = State_Timeouting;
			//state = State_Shutter_Closing;
			break;
		case State_Timeouting:
		/* Once the shutter relase and the focus button are relased state is switched to State_Initialize. This is done to prevent immediate retriggering of the shutter once state is set to State_Idle later. Also blink the power LED to make error state known.
		*/
			if (g_error_state_blink == 1) {
				PORTD ^= (1<<PD6); // Toggle power indicator LED
				g_error_state_blink = ERROR_BLINK; //restart timer
			}
			if (!BTN_RELEASE && !BTN_PREVIEW) {
				state = State_Initialize; // Wait for Shutter to be realeased
				g_error_state_blink = 0;
			}
			break;
		default:
			state = State_Idle;
			break;
	}	
//	finished = true;
}

/* This function is executed every tick independent of the state 
*/
void State_Independent(void) {
	/* Count down various delay ticks, see global.h for details.
	*/
	if (g_tick_debounce > 0 ) {
		g_tick_debounce--;
	}	
	if (g_tick_close_delay > 0) {
		g_tick_close_delay--;
	}
	if (g_tick_init > 0) {
		g_tick_init --;
	}
	if (g_tick_shutter > 0) {
		g_tick_shutter--;
	}

	/* Cound only when value is greater than one. This way it's possible to set the timer a sudo disabled state
	*/
	if (g_tick_pwm > 1 ) {
		g_tick_pwm--;
	}
	if (g_tick_pwm == 1 && g_shutter_is_activated) {
		Enable_PWM();
		g_tick_pwm = 0; // Prevent reenabling
	}

	//tick for flash output duration
	if (g_tick_flash_duration > 1) {
		g_tick_flash_duration--;
	}
	if (g_tick_flash_duration == 1) {
		OUTPUT_PORT &= ~(1 << FLASH_OUT_PIN); // Disable flash output
		g_tick_flash_duration = 0; 
	}

	// Special case: Timeout, state gets changed!
	if (g_shutter_opening_timeout > 1) {
		g_shutter_opening_timeout--;
	}
	if (g_shutter_opening_timeout == 1) {
		g_shutter_opening_timeout = 0; // Disable timeout 
		state = State_Timeout; // Go to timeout state 
	}
	if (g_error_state_blink > 1) {
		g_error_state_blink --;
	}
}