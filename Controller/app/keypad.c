/**
* @file
* @brief Keypad reading functionality
*
*/
#include "../src/keypad.h"
#include <msp430.h>

// reversed to match datasheet to pin indices
char key_chars[4][4] = {
    {'D', '#', '0', '*'},  
    {'C', '9', '8', '7'},  
    {'B', '6', '5', '4'},  
    {'A', '3', '2', '1'}  
};

/* Initialize keypad GPIO pins*/
void init_keypad(Keypad *keypad) {
    // set keypad row pins as input
    int i;
    for (i = 0; i < 4; i++){
        P5DIR &= ~(keypad->row_pins[i]);        // set pin to input
        P5REN |= keypad->row_pins[i];         // enable pull up/down resistor
        P5OUT |= keypad->row_pins[i];         // set pull up resistor
    }

    // set keypad col pins as output
    for (i = 0; i < 4; i++){
        P2DIR |= keypad->col_pins[i];         // set pin to output
        P2OUT |= keypad->col_pins[i];         // set high
    }
}

/* check for a button press by setting a column low, then checking which row is also low */
int scan_keypad(Keypad *keypad, char *key_press) {
    // for each col, check if there is a LOW row
    int col, row;

    for(col = 0; col < 4; col++){
        // clear, set col LOW
        P2OUT &= ~keypad->col_pins[col];
        __delay_cycles(1000);

        for(row = 0; row < 4; row++){
            if (!(P5IN & keypad->row_pins[row])){
                *key_press = key_chars[row][col];
                // set col HIGH
                P2OUT |= keypad->col_pins[col];
                return SUCCESS;
            }
        }
        // set col HIGH
        P2OUT |= keypad->col_pins[col];
    }
    return FAILURE;
}