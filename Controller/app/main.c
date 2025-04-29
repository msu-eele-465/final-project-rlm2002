/**
* @file
* @brief LCD interface functionality
*
*/
#include <msp430.h>
#include "intrinsics.h"
#include "src/keypad.h"
#include "msp430fr2355.h"
#include "src/led.h"

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

int main(void) {

    // Disable watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

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

    // Disable the GPIO power-on default high-impedance mode
    // to activate previously configured port settings
    __enable_interrupt();   // enable maskable IRQs
    PM5CTL0 &= ~LOCKLPM5;   // turn on GPIO

    init_LED(&led1);

    while(1)
    {
    }
}
