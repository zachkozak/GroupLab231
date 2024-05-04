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


const unsigned int on_period = 500;          // a made-up number, probably not the value you want to use
volatile unsigned int total_period = 50000;
volatile bool sound_alarm;
volatile bool light_LED = false;
volatile unsigned int alarm_interrupt_ct;
volatile bool alarm_requested;

void set_total_period(void){
    if(status == normal){
        if(distance >= 250) total_period = 3500;
        else if(distance >= 200) total_period = 3000;
        else if(distance >= 100) total_period = 2500;
        else if(distance >= 50) total_period = 2000;
        else if(distance >= 25) total_period = 1500;
        else if(distance >= 10) total_period = 1000;
        else if(distance >= 5) total_period = 750;
        else total_period = 500;
    } else total_period = 50000;   
}

void handle_alarm_timer_interrupt(void){
    static bool on;
    if(sound_alarm && (alarm_interrupt_ct <= on_period)){
        if (on) {
            digitalWrite(BUZZER, HIGH);
            on = false;
        } else {
            digitalWrite(BUZZER, LOW);
            on = true;
        }
    } 

    if(light_LED){
        cowpi_illuminate_right_led();
        light_LED = false;
    } else {
        cowpi_deluminate_right_led();
    }

    if(alarm_interrupt_ct > on_period){
        sound_alarm = false;
    }

    alarm_interrupt_ct++;
    if(alarm_interrupt_ct >= total_period){
        alarm_interrupt_ct = 0;
        if(object_detected && status == normal){
            sound_alarm = true;
            light_LED = true;
        }
    }
    
    //reset_timer(ALARM_TIMER);
}

void initialize_alarm(void) {
    register_timer_ISR(ALARM_TIMER, 500, handle_alarm_timer_interrupt);
}

void manage_alarm(void) {
    set_total_period();
    if(status == continuous_tone){
        sound_alarm = true;
        alarm_interrupt_ct = 0;
    } else if (status == single_pulse){
        if(alarm_requested){
            sound_alarm = true;
            light_LED = true;
            alarm_requested = false;
            alarm_interrupt_ct = 0;
        }
    } else if (status == thresh_adjust){
        sound_alarm = false;
    }
}
