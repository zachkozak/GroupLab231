/*                       *
 * DO NOT EDIT THIS FILE *
 *                       */

/**************************************************************************//**
 *
 * @file outputs.cpp
 *
 * @author Christopher A. Bohn
 *
 * @brief Functions to place text on the display module.
 *
 ******************************************************************************/

/*
 * RangeFinder GroupLab (c) 2023-24 Christopher A. Bohn
 *
 * Assignment and starter code licensed under the Apache License,
 * Version 2.0 (http://www.apache.org/licenses/LICENSE-2.0).
 */

#include <CowPi.h>
#include <string.h>
#if defined (ARDUINO_RASPBERRY_PI_PICO)
#include <OneBitDisplay.h>
#endif //PICO
#include "outputs.h"

#if defined (ARDUINO_AVR_NANO)
static FILE *display;
#elif defined (ARDUINO_RASPBERRY_PI_PICO)
static uint8_t backbuffer[1024] = {0};
static OBDISP display;
static void initialize_ssd1306(void);
#else
#error Unknown board!
#endif // WHICH BOARD?

static bool display_needs_refreshed;


/**
 * Configures the display module, along with the rest of the Cow Pi hardware.
 */
void initialize_board(void) {
#if defined (ARDUINO_AVR_NANO)
    display = cowpi_setup(9600,
                          cowpi_configure_lcd_character_display(),
                          cowpi_configure_i2c(cowpi_discover_i2c_address(SDA, SCL))
    );
    if (display == NULL) {
        int i2c_configuration_line = __LINE__ - 3;
        fprintf_P(stderr, PSTR("Display was not configured!\n ***"));
        int8_t i2c_address = cowpi_discover_i2c_address(SDA, SCL);
        if (i2c_address == 0) {
            fprintf_P(stderr, PSTR("No I2C device detected.\n"));
        } else if (i2c_address == -1) {
            fprintf_P(stderr, PSTR("Multiple I2C devices detected.\n"));
            fprintf_P(stderr, PSTR("Modify line %d to hard-code the correct I2C address.\n"), i2c_configuration_line);
        } else {
            fprintf_P(stderr, PSTR("This does not appear to be an I2C problem.\n"));
        }
        fprintf_P(stderr, PSTR("\nRe-assigning display module's output to the console\n"));
        fprintf_P(stderr, PSTR("so that you can work on other parts of the assignment.\n\n"));
        display = stdout;
    }
#elif defined (ARDUINO_RASPBERRY_PI_PICO)
    cowpi_setup(0,
            (cowpi_display_module_t) {.display_module = NO_MODULE},
            (cowpi_display_module_protocol_t) {.protocol = NO_PROTOCOL}
    );
    initialize_ssd1306();
#endif // WHICH BOARD?
}


#if defined (ARDUINO_AVR_NANO)
static int const NUMBER_OF_ROWS = 2;
static char rows[2][17] = {
        {"                "}, {"                "}
};
#elif defined (ARDUINO_RASPBERRY_PI_PICO)
static int const NUMBER_OF_ROWS = 8;
static char rows[8][17] = {
        {"                "}, {"                "}, {"                "}, {"                "},
        {"                "}, {"                "}, {"                "}, {"                "}
};

static void initialize_ssd1306(void) {
    obdI2CInit(&display, OLED_128x64, -1, 0, 0, 1, -1, -1, -1, 400000L);
    obdSetBackBuffer(&display, backbuffer);
    obdFill(&display, OBD_WHITE, 0);
    display_needs_refreshed = true;
}
#endif // WHICH BOARD?


void display_string(int row, char const string[]) {
    if (0 <= row && row < NUMBER_OF_ROWS) {
        sprintf(rows[row], "%-16s", string);
    }
#if defined (ARDUINO_RASPBERRY_PI_PICO)
    obdWriteString(&display, 0, 0, 8 * row, rows[row], FONT_8x8, OBD_BLACK, 0);
#endif //PICO
    display_needs_refreshed = true;
}

void refresh_display(void) {
    if (display_needs_refreshed) {
#if defined (ARDUINO_AVR_NANO)
        fprintf(display, "\f%s\n%s\f", rows[0], rows[1]);
#elif defined (ARDUINO_RASPBERRY_PI_PICO)
        obdDumpBuffer(&display, backbuffer);
#endif // WHICH BOARD?
        display_needs_refreshed = false;
    }
}

