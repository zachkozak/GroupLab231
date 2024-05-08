/**************************************************************************//**
 *
 * @file user_controls.c
 *
 * @author Zach Kozak Jack Williames
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
volatile status_t status;
volatile int32_t number = 0;


void initialize_controls(void) {
    requested_ping = false;
}


int get_working_key(char key_face) {
    switch (key_face) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return -1;
    }
}


void manage_controls(void) {
    bool left_switch = cowpi_debounce_byte(cowpi_left_switch_is_in_left_position(), LEFT_SWITCH_LEFT);
    bool right_switch = cowpi_debounce_byte(cowpi_right_switch_is_in_left_position(), RIGHT_SWITCH_LEFT);
    if (left_switch && right_switch) status = normal;
    else if (left_switch && !right_switch) status = continuous_tone;
    else if (!left_switch && right_switch) status = single_pulse;
    else status = thresh_adjust;

    if(status == normal) display_string(2, "Normal");
    else if(status == continuous_tone) display_string(2, "Continuous Tone");
    else if(status == single_pulse) display_string(2, "Single Pulse");
    else display_string(2, "Adjust Threshold");
    
    static bool last_left_button_state;
    bool left_button = cowpi_debounce_byte(cowpi_left_button_is_pressed(), LEFT_BUTTON_DOWN);
    if(status == single_pulse){
        if(left_button != last_left_button_state) {
            last_left_button_state = left_button;
            requested_ping = left_button;
        }
    }

    if(status == thresh_adjust){
        display_string(0,"");
        display_string(1,"");
        static bool outofrange;
        static char lastkey;
        char key = cowpi_debounce_byte(cowpi_get_keypress(), KEYPAD);
        if(key != lastkey){
            lastkey = key;
            if(key == '#'){
                if(number >= 50 && number <= 400){
                    threshold = number;
                    number = 0;
                    outofrange = false;
                } else {
                    number = 0;
                    outofrange = true;
                }
            } else {
                int key_value = get_working_key(key);
                if(key_value >= 0){
                    number = number * 10 + key_value;
                }
                if(number > INT32_MAX || number < 0){
                    number = 0;
                }
            }
        }
        char buffer[16];
        sprintf(buffer, "%12ld cm", number);
        display_string(4, buffer);

        if(outofrange){
            display_string(5, "Must Be In Range");
            display_string(6, "       50-400 cm");
        } else {
            char buffer2[17];
            sprintf(buffer2, "%12ld cm", (int32_t) threshold);
            display_string(5, "Current:");
            display_string(6, buffer2);
        }
    } else {
        display_string(4, "");
        display_string(5, "");
        display_string(6, "");
    }

    
}
