/* --COPYRIGHT--,BSD
 * Copyright (c) 2018, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/
//***************************************************************************************
//  Blink the LED Demo - Software Toggle P1.0
//
//  Description; Toggle P1.0 inside of a software loop using DriverLib.
//  ACLK = n/a, MCLK = SMCLK = default DCO
//
//                MSP4302355
//             -----------------
//         /|\|              XIN|-
//          | |                 |
//          --|RST          XOUT|-
//            |                 |
//            |             P1.0|-->LED
//
//  E. Chen
//  Texas Instruments, Inc
//  May 2018
//  Built with Code Composer Studio v8
//***************************************************************************************

// PERSISTENT stores vars in FRAM so they stick around through power cycles
__attribute__((persistent)) static status_LED led1 =
{
    .led_port_base_addr = P5_BASE,
    .red_port_bit = BIT0,
    .green_port_bit = BIT1,
    .blue_port_bit = BIT2,
    .current_state = LEDLOCKED
};
__attribute__((persistent)) static status_LED led2 =
{
    .led_port_base_addr = P5_BASE,
    .red_port_bit = BIT0,
    .green_port_bit = BIT1,
    .blue_port_bit = BIT2,
    .current_state = LEDLOCKED
};
__attribute__((persistent)) static status_LED led1 =
{
    .led_port_base_addr = P5_BASE,
    .red_port_bit = BIT0,
    .green_port_bit = BIT1,
    .blue_port_bit = BIT2,
    .current_state = LEDLOCKED
};

int main(void) {

    volatile uint32_t i;

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

    while(1)
    {
        // Toggle P1.0 output
        GPIO_toggleOutputOnPin(
            GPIO_PORT_P1,
            GPIO_PIN0
            );

        // Delay
        for(i=10000; i>0; i--);
    }
}
