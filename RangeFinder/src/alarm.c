/**************************************************************************//**
 *
 * @file alarm.c
 *
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 *
 * @brief Code to manage the piezodisc and LEDs.
 *
 ******************************************************************************/

/*
 * RangeFinder GroupLab assignment and starter code (c) 2023 Christopher A. Bohn
 * RangeFinder solution (c) the above-named students
 */

#include <CowPi.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "alarm.h"
#include "shared_variables.h"
#include "outputs.h"
#include "interrupt_support.h"


const unsigned int on_period = 7000;          // a made-up number, probably not the value you want to use
volatile unsigned int total_period = 50000;
volatile bool sound_alarm;
volatile bool on = false;
volatile unsigned int alarm_interrupt_ct = 0;

void handle_alarm_timer_interrupt(void){
    alarm_interrupt_ct++;
    if(alarm_interrupt_ct >= total_period){
        alarm_interrupt_ct = 0;
    }
    if (on) {
        digitalWrite(BUZZER, HIGH);
        on = false;
    } else {
        digitalWrite(BUZZER, LOW);
        on = true;
    }
    reset_timer(ALARM_TIMER);
}

void initialize_alarm(void) {
    register_timer_ISR(ALARM_TIMER, 500, handle_alarm_timer_interrupt);
}

void manage_alarm(void) {
  
}
