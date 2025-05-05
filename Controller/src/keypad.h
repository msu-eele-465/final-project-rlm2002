/**
* @file
* @brief Header file for keypad reading and password management
*
* Uses PWMs through 3 arbitrary RGB outputs to change an LED's color
*/
#ifndef  KEYPAD_H
#define KEYPAD_H

#include <msp430.h>
#include <stdio.h>

#ifndef LOCKED
#define LOCKED 1
#endif
#ifndef UNLOCKED
#define UNLOCKED 0
#endif
#ifndef SUCCESS
#define SUCCESS 1
#endif
#ifndef FAILURE
#define FAILURE 0
#endif

/**
* game state definitions
*/
typedef enum
{
    IDLE,           // no game data needs to be saved. set everything to starting value
    IN_PROGRESS,    // first button press 
    CHECK,          // button press to check answer
    WIN,            // if check is good
    LOSE            // if win is not acheived within 30s
} game_state;

/**
* row and column pins, passkey, and current state
*/
typedef struct {
    int row_pins[4];      // order is 5, 6, 7, 8
    int col_pins[4];      // order is 1, 2, 3, 4
} Keypad;

extern char key_chars [4][4];

/**
* initializes keypad GPIO pins
* 
* @param: keypad constructor.
*
* @return: NA
*/
void init_keypad(Keypad *keypad);

/**
* check for a button press by setting a column low, then checking which row is also low
* 
* @param: keypad constructor.
* @param: pointer to pressed key.
*
* @return: SUCCESS or FAILURE
*/
int scan_keypad(Keypad *keypad, char *key_press);

#endif
