/**
* @file
* @brief main controller for game
*
*/
#include <msp430.h>
#include "intrinsics.h"
#include "src/keypad.h"
#include "msp430fr2355.h"
#include "src/led.h"

game_state current_game_state = IDLE;
uint8_t ten_sec = 0, ones_sec = 0;          // tens place and ones place for timer
uint8_t value = 0;
char cur_char = 'x';

// PERSISTENT stores vars in FRAM so they stick around through power cycles
__attribute__((persistent)) static rgb_LED led1 =
{
    .red_port_bit = BIT0,
    .green_port_bit = BIT1,
    .blue_port_bit = BIT2,
    .current_state = OFF
};
__attribute__((persistent)) static rgb_LED led2 =
{
    .red_port_bit = BIT3,
    .green_port_bit = BIT4,
    .blue_port_bit = BIT5,
    .current_state = OFF
};

Keypad keypad = {
    .lock_state = LOCKED,                       // locked is 1
    .row_pins = {BIT3, BIT2, BIT1, BIT0},       // order is 5, 6, 7, 8
    .col_pins = {BIT0, BIT1, BIT2, BIT3},       // order is 1, 2, 3, 4
};

void init(){
    
    // Disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // Disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;
    
    // set up ports
    // heartbeat led
    P1DIR |= BIT0;              // Config as Output
    P1OUT |= BIT0;              // turn on to start

    // button
    P4DIR &= ~BIT1;             // Configure P4.1 as input
    P4REN |= BIT1;              // Enable resistor
    P4OUT &= ~ BIT1;              // Make pull down resistor
    P4IES |= BIT1;              // Configure IRQ sensitivity H-to-L

    // dipswitch
    P3DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);      // set pins to output
    P3REN |= BIT0 | BIT1 | BIT2 | BIT3;         // enable resistor
    P3OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);      // pull down resistor

    // set up IRQ
    P4IFG &= ~BIT1;             // Clear P4.1 IRQ Flag
    P4IE |= BIT1;               // Enable P4.1 IRQ

    // Timer B0
    // Math: 1s = (1*10^-6)(D1)(D2)(25k)    D1 = 5, D2 = 8
    TB0CTL |= TBCLR;            // Clear timer and dividers
    TB0CTL |= TBSSEL__SMCLK;    // Source = SMCLK
    TB0CTL |= MC__UP;           // Mode UP
    TB0CTL |= ID__8;            // divide by 8 
    TB0EX0 |= TBIDEX__5;        // divide by 5 (100)


    TB0CCR0 = 25000;

    TB0CCTL0 &= ~CCIFG;         // Clear CCR0
    TB0CCTL0 |= CCIE;           // Enable IRQ

    __enable_interrupt();   // enable maskable IRQs
    PM5CTL0 &= ~LOCKLPM5;   // turn on GPIO

    init_LED(&led1);
}

uint8_t get_dipswitch() {
    return P3IN & 0x0F;
}

int main(void) {
    init();

    while(1)
    {
        // if (current_game_state == IN_PROGRESS) {
        //     ret = scan_keypad(&ke, char *key_press)
        // }
        value = value;
        __delay_cycles(100000);
    }
}


// -- Interrupt Service Routines ------------------------ //

/**
* Heartbeat LED, read time every 1s
*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void heartbeat_LED(void)
{
    P1OUT ^= BIT0;          // LED1 xOR
    
    TB1CCTL0 &= ~CCIFG;     // clear flag
}
// ----- end heartbeat_LED-----

/*
 * Handles changes in state due to button press
 */
#pragma vector = PORT4_VECTOR
__interrupt void get_button_press(void)
{
    if (current_game_state == IDLE) {
        current_game_state = IN_PROGRESS;
    } else if (current_game_state == IN_PROGRESS) {
        current_game_state = CHECK;
    }
    P4IFG &= ~BIT1;
}
// ----- end get_button_press-----
