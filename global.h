/*
 * global.h
 *
 * Created: 20.05.2024
 *  Author: Elektronaut
 */ 

#ifndef GLOBAL_H_
#define GLOBAL_H_

typedef enum {State_Idle = 1, State_Preview, State_Preview_Open, State_Toggle, State_Toggle_Open, State_Toggle_Btn_Pressed, State_Bulb, State_Bulb_Open, State_Shutter_Closing, State_Shutter_Closed, State_Timer, State_Timer_Opening, State_Timer_Open, State_Initialize, State_Initializing, State_Timeout, State_Timeouting} _states;
typedef enum {t1_60 = 1, t1_30, t1_15, t1_8, t1_4, t1_2, t1, t2, t5, t10, Mode_Bulb, Mode_Toggle } _modes;

volatile _states state;
volatile _modes g_mode_switch; 
volatile bool g_shutter_is_activated;

volatile bool g_flash_has_fired;
volatile bool g_flash_enabled;

volatile uint8_t g_buttons;
volatile uint8_t g_tick_flash_delay;  
volatile uint8_t g_tick_flash_duration;  // Tick for duration how long to enable flash output
volatile uint8_t g_tick_init; // Tick for initializing delay
volatile uint8_t g_tick_debounce; // Tick for button debounce delay

volatile uint16_t g_tick_pwm; // Tick for delay to enable holding current reduction
volatile uint16_t g_tick_close_delay; // Tick for delay after the shutter is switched off. Time to close the shutter
volatile uint16_t g_tick_shutter; // Tick for main timer for timed exposures!
volatile uint16_t g_shutter_opening_timeout;  // Tick for timeout in case the shutter cant open to reset the state and return to State_Idle
volatile uint16_t g_error_state_blink;

#endif