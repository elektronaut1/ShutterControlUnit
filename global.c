/*
 * global.c
 *
 * Created: 20.05.2024
 *  Author: Elektronaut
 */ 

#include "includes.h"

volatile _states state;
volatile _states nextState;
volatile _modes g_mode_switch; 

volatile bool g_shutter_is_activated = false;
volatile bool g_flash_has_fired = false;

volatile uint8_t g_buttons = 0x00;
volatile uint8_t g_tick_flash_delay = 0;
volatile uint8_t g_tick_debounce = 0; 
volatile uint8_t g_tick_flash_duration = 0;
volatile uint8_t g_tick_init = 0;

volatile uint16_t g_tick_close_delay = 0;
volatile uint16_t g_tick_pwm = 0;
volatile uint16_t g_tick_shutter = 0;
volatile uint16_t g_error_state_blink = 0;