/**************************************************************************//**
 *
 * @file user_controls.c
 *
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 * @author (STUDENTS -- TYPE YOUR NAMES HERE)
 *
 * @brief Code to get inputs from the user.
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
#include <string.h>
#include <stdlib.h>
#include "user_controls.h"
#include "shared_variables.h"
#include "outputs.h"
#include "interrupt_support.h"
volatile bool requested_ping;
volatile enum {
    continuous_tone,
    normal,
    single_pulse,
    thresh_adjust
} status = normal;


void initialize_controls(void) {
    requested_ping = false;
}

void manage_controls(void) {
    if (cowpi_left_switch_is_in_left_position() && cowpi_right_switch_is_in_left_position()) status = normal;
    else if (cowpi_left_switch_is_in_left_position() && !cowpi_right_switch_is_in_left_position()) status = continuous_tone;
    else if (!cowpi_left_switch_is_in_left_position() && cowpi_right_switch_is_in_left_position()) status = single_pulse;
    else status = thresh_adjust;
    if(status == normal) display_string(2, "Normal");
    else if(status == continuous_tone) display_string(2, "Continuous Tone");
    else if(status == single_pulse) display_string(2, "Single Pulse");
    else display_string(2, "Threshold Adjust");
    
    if(status == single_pulse){
        if(cowpi_left_button_is_pressed() && !requested_ping){
            requested_ping = true;
        }
    }

    
}
