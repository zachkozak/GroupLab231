/**************************************************************************//**
 *
 * @file sensor.c
 *
 * @author Zach Kozak Jack Williames
 *
 * @brief Code to manage the distance sensor.
 *
 ******************************************************************************/

/*
 * RangeFinder GroupLab assignment and starter code (c) 2023 Christopher A. Bohn
 * RangeFinder solution (c) the above-named students
 */

#include <CowPi.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "sensor.h"
#include "shared_variables.h"
#include "outputs.h"
#include "interrupt_support.h"

volatile bool object_detected;
volatile int64_t distance;
volatile int64_t last_distance;
volatile int64_t current_distance;

volatile int64_t speed;
volatile int64_t threshold = 400;
volatile adc_t *adc = (adc_t *)0x4004c000;
volatile enum {
    STATE_INITIAL_START,
    STATE_POWERING_UP,
    STATE_READY,
    STATE_ACTIVE_LISTENING,
    STATE_ACTIVE_DETECTED,
    STATE_QUIESCENT
} sensorState;
static cowpi_timer_t volatile *timer = (cowpi_timer_t *) (0x40054000);
volatile int64_t last_time;
volatile int64_t current_time;
volatile uint32_t speedtimer;

void handle_sensor_timer_interrupt(void){
    
    if(sensorState == STATE_INITIAL_START){
        sensorState = STATE_POWERING_UP;
        reset_timer(SENSOR_TIMER);
    } else if(sensorState == STATE_POWERING_UP){
        sensorState = STATE_READY;
        reset_timer(SENSOR_TIMER);
    } else if(sensorState == STATE_ACTIVE_LISTENING){
        object_detected = false;
        sensorState = STATE_QUIESCENT;
        reset_timer(SENSOR_TIMER);
    } else if(sensorState == STATE_ACTIVE_DETECTED){
        object_detected = true;
        if(status == single_pulse){
            if(distance <= threshold){
                alarm_requested = true;
            } else light_LED = true;
        }
        sensorState = STATE_QUIESCENT;
        reset_timer(SENSOR_TIMER);
    } else if(sensorState == STATE_QUIESCENT){
        sensorState = STATE_READY;
        reset_timer(SENSOR_TIMER);
    } else if(sensorState == STATE_READY){
        if(status == normal){
            requested_ping = true;
        }
    }
}

void handle_rise_and_fall_interrupt(void){
    if(sensorState == STATE_ACTIVE_LISTENING){
        current_time = timer->raw_lower_word;
        current_time = current_time - last_time;
        sensorState = STATE_ACTIVE_DETECTED;
    } else if (sensorState == STATE_READY){
        reset_timer(SENSOR_TIMER);
        sensorState = STATE_ACTIVE_LISTENING;
        last_time = timer->raw_lower_word;
    }
}

void initialize_sensor(void) {
    adc->control |= (1 << 20);
    adc->control |= (4 << 12);
    adc->control |= (3);
    sensorState = STATE_INITIAL_START;
    object_detected = false;
    speed = 0;
    register_timer_ISR(SENSOR_TIMER, 32768, handle_sensor_timer_interrupt);
    register_pin_ISR((1<<ECHO), handle_rise_and_fall_interrupt);
}

void manage_sensor(void) {
    if(requested_ping){
        
        digitalWrite(TRIGGER, HIGH);
        requested_ping = false;
        uint32_t time = timer->raw_lower_word;
        while(timer->raw_lower_word - time < 10){}
        digitalWrite(TRIGGER, LOW);
    }
    if(object_detected && status != thresh_adjust){
        //Read Temperature:
        adc->control |= (4 << 12);
        adc->control |= 4;
        while ((adc->control & (1 << 8)) == 0) {}
        uint32_t temp = adc->result;
        //Not sure if the calculation needs distance or times
        distance = current_time * (256108888 - (121907 * (temp)));
        distance = distance >> 33;
        if (object_detected) {
            // Read Temperature:
            adc->control |= (4 << 12);
            adc->control |= 4;
            while ((adc->control & (1 << 8)) == 0) {}
            uint32_t temp = adc->result;
            
            distance = current_time;
            distance *= (256108888 - (121907 * temp));
            distance = distance >> 33;

            static char buff[17];
            snprintf(buff, sizeof(buff), "%13lu cm", (uint32_t)distance);
            display_string(0, buff);
            display_string(1, "Object Detected");

            if(status == normal){
                if(timer->raw_lower_word - speedtimer > 1000000){
                    speedtimer = timer->raw_lower_word;
                    last_distance = current_distance;
                    current_distance = distance;
                    speed = (last_distance - current_distance);
                }
                snprintf(buff, sizeof(buff), "%11ld cm/s", (int32_t) speed);
                display_string(3, buff);
            } else {
                display_string(3, "");
            }
        }
    } else {
        display_string(0,"No Object Detect");
        display_string(1,"");
        display_string(3,"");
    }

}
