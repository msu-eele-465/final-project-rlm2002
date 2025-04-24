/**
* @file
* @brief Header file for led_status.c
*
* Uses PWMs through 3 arbitrary RGB outputs to change an LED's color
*/
#ifndef LED_H
#define LED_H

#include <stdint.h>

/**
* state definitions for LEDs
*/
typedef enum
{
    LEDLOCKED,
    COLOR1,
    COLOR2,
    COLOR3,
    COLOR4,
    COLOR5,
    COLOR6,
    COLOR7,
    COLOR8,
    COLOR9,
    COLOR0,
} LED_State;

/**
* holds port addresses and bits for RGB outputs plus current state
*/
typedef struct rgb_LED 
{ 
    const uint16_t led_port_base_addr;
    const uint16_t red_port_bit;
    const uint16_t green_port_bit;
    const uint16_t blue_port_bit;
    LED_State current_state;
} status_LED;

/**
* initializes LED and starts the timer compares
* 
* @param: pointer to the status_LED struct.
* @param: s current state.
*
* @return: NA
*/
void init_LED(status_LED *sL);

/**
* Sets LEDs with right colors
*
* Uses states to determine correct color to set LED
* pulses LED at correct % (x/255) to attain the color
* 
* @param: sL constructor.
* @param: s current state.
*
* @return: NA
*/
void set_LED(status_LED *sL, LED_State s);

#endif // LED_STATUS_H