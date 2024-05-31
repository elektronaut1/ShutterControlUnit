/*
 * modules.h
 *
 * Created: 20.05.2024 01:30:40
 *  Author: Elektronaut
 */ 


#ifndef MODULES_H_
#define MODULES_H_

void Read_rotary_switch(void);
void Enable_PC_PCINT(void);
void Disable_PC_PCINT(void);
void Shutter_Close(void);
void Shutter_Open(void);
void Shutter_open_with_flash_with_pwm(void);
void Shutter_OpenAndStartTimer(void);

void Shutter_open_with_pwm(void);
void Read_Inputs(void);

uint16_t Read_ADC(uint8_t);
void Setup_System(void);

void Enable_PWM(void);
void Disable_PWM(void);

#ifdef STATEMACHINE_USE_TIMER_2
void setup_timer2(void);
#endif

#ifdef STATEMACHINE_USE_TIMER_1
void setup_timer1(void);
#endif

#endif /* MODULES_H_ */