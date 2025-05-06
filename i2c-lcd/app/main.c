/**
* @file
* @brief Slave code for LCD msp. Recieves data and displays a number.
*
*/
#include <msp430fr2310.h>
#include <stdbool.h>
#include <stdint.h>
#include "src/lcd.h"


volatile char rx_data[32];
uint8_t idx = 0;
uint8_t received_data = 0, data_received = 0;
uint8_t data_received_count;

int main(void)
{
    // Stop watchdog timer
    WDTCTL = WDTPW | WDTHOLD;

    // I2C Setup
    UCB0CTLW0 &=~UCSWRST;                                 //clear reset register

    // 1. Put eUSCI_B0 into software reset
    UCB0CTLW0 |= UCSWRST;        // UCSWRST = 1 for eUSCI_B0 in SW reset

    // 2. Configure eUSCI_B0
    UCB0CTLW0 |= UCMODE_3;                //I2C slave mode, SMCLK
    UCB0I2COA0 = 0x0A | UCOAEN;           //SLAVE0 own address is 0x0A| enable

    // 3. Configure Ports as I2C
    P1SEL1 &= ~BIT3;            // P1.3 = SCL
    P1SEL0 |= BIT3;

    P1SEL1 &= ~BIT2;            // P1.2 = SDA
    P1SEL0 |= BIT2;

    // 4. Take eUSCI_B0 out of SW reset
    UCB0CTLW0 &= ~UCSWRST;

    // 5. Enable Interrupts
    UCB0IE |= UCRXIE0 | UCSTPIE;          // Enable I2C Rx0 IRQ

     // Timer B0
    // Math: 1s = (1*10^-6)(D1)(D2)(25k)    D1 = 5, D2 = 8
    TB0CTL |= TBCLR;        // Clear timer and dividers
    TB0CTL |= TBSSEL__SMCLK;  // Source = SMCLK
    TB0CTL |= MC__UP;       // Mode UP
    TB0CTL |= ID__8;         // divide by 8 
    TB0EX0 |= TBIDEX__5;    // divide by 5 (100)

    TB0CCR0 = 25000;

    TB0CCTL0 &= ~CCIFG;     // Clear CCR0
    TB0CCTL0 |= CCIE;       // Enable IRQ

    // LED Setup
    P2DIR |= BIT0;          // Config as Output
    P2OUT |= BIT0;          // turn on to start

    // variable initiation

    __enable_interrupt();       // Enable Maskable IRQs

    init_lcd();

    // Disable low-power mode / GPIO high-impedance
    PM5CTL0 &= ~LOCKLPM5;

    while (true)
    {
        if (received_data) {
            uint8_t i;
            lcd_send_command(LCD_RETURN_HOME);
            for (i = 0; i < 16; i++){
                lcd_send_data(rx_data[i]);
            }
            DELAY_0001;
            lcd_send_command(LCD_BOTTOM_LINE);
            for (i = 16; i < 32; i++){
                lcd_send_data(rx_data[i]);
            }
            received_data = 0;
        }
    }
}


//-- Interrupt Service Routines -----------------------

/**
* receiveData
*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void receive_data(void)
{
    switch(UCB0IV)             // determines which IFG has been triggered
    {
        case USCI_I2C_UCRXIFG0:                 // ID 0x16: Rx IFG
            if (idx < 32) {
                rx_data[idx] = UCB0RXBUF;                   // retrieve data
                idx++;
            }
            break;
        case USCI_I2C_UCSTPIFG:            // Stop condition
            received_data = 1;
            data_received = 1;
            idx = 0;                  // Reset index for next reception
            UCB0IFG &= ~UCSTPIFG;
            break;
        default:
            break;
    }

}
//----- end receiveData------------

/**
* Heartbeat LED
*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void heartbeat_LED(void)
{
    P2OUT ^= BIT0;          // P2.0 xOR

    TB0CCTL0 &= ~CCIFG;     // clear flag
}
// ----- end heartbeatLED-----
