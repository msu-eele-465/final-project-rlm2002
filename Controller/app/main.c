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

volatile game_state current_game_state = IDLE;
volatile uint8_t seconds = 30;          // timer
volatile uint8_t tensecs = 3;
volatile uint8_t secs = 0;
volatile uint8_t tx_byte_cnt = 0, index1 = 0, index2 = 0;
char cur_char = 'x';
volatile uint8_t first_led_set = 0;
volatile uint8_t trigger_buzzer = 0;
volatile uint8_t update_lcd = 0;

uint8_t secret_pin[] = {RED, RED, 1};
uint8_t pin[] = {0, 0, 0};

char *lcd_line1_strings[] = {" Push to Start ", "Guess the Code ",
                "  Checking...  ", "    WINNER   ", "   Game Over   "};
char lcd_line2[] = "00s            ";

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
    .row_pins = {BIT3, BIT2, BIT1, BIT0},       // order is 5, 6, 7, 8
    .col_pins = {BIT3, BIT2, BIT1, BIT0},       // order is 1, 2, 3, 4
};

/**
* inits lcd transmit
*/
void transmit_to_lcd()
{
    index1 = 0;
    index2 = 0;
    tx_byte_cnt = 32;
    while (UCB0CTLW0 & UCTXSTP);                      // Ensure stop condition got sent
    UCB0CTLW0 |= UCTR | UCTXSTT;                      // I2C TX, start condition
}
/**
* inits pins, leds, and keypad
*/
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
    P4OUT &= ~BIT1;              // Make pull down resistor (pull up if using dev board)
    P4IES |= BIT1;              // Configure IRQ sensitivity H-to-L

    // dipswitch
    P3DIR &= ~(BIT0 | BIT1 | BIT2 | BIT3);      // set pins to output
    P3REN |= BIT0 | BIT1 | BIT2 | BIT3;         // enable resistor
    P3OUT &= ~(BIT0 | BIT1 | BIT2 | BIT3);      // pull down resistor

    // buzzer
    P2DIR |= BIT5;              // P2.5 to output
    P2OUT &= ~BIT5;             // off to start
    P2IFG &= ~BIT5;             // Clear P2.3 IRQ Flag
    P2IE |= BIT5;               // Enable P2.3 IRQ

    // set up IRQ
    P4IFG &= ~BIT1;             // Clear P4.1 IRQ Flag
    P4IE |= BIT1;               // Enable P4.1 IRQ

    // Configure Pins for I2C
    P1SEL1 &= ~BIT3;            // P1.3 = SCL
    P1SEL1 &= ~BIT2;            // P1.2 = SDA
    P1SEL0 |= BIT2 | BIT3;                            // I2C pins

    // Configure USCI_B0 for I2C mode
    UCB0CTLW0 |= UCSWRST;                             // put eUSCI_B in reset state
    UCB0CTLW0 |= UCMODE_3 | UCMST;                    // I2C master mode, SMCLK
    UCB0CTLW1 |= UCASTP_2;                            // Automatic stop after hit TBCNT
    UCB0I2CSA = 0x0A;                         // configure slave address
    UCB0BRW = 0x8;                                    // baudrate = SMCLK / 8
    UCB0TBCNT = 0;                                    // num bytes to recieve

    UCB0CTLW0 &=~ UCSWRST;                            // clear reset register
    UCB0IE |= UCTXIE0 | UCNACKIE | UCRXIE0 | UCBCNTIE;// transmit, receive, TBCNT, and NACK

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

    init_LED(&led1, 0);
    init_LED(&led2, 1);
    init_keypad(&keypad);

    // At the end of init:
    __delay_cycles(100000);  // Give LCD time to initialize
    transmit_to_lcd();
}

/**
* get integer value of dipswitch
*/
uint8_t get_dipswitch() {
    return P3IN & 0x0F;
}

int main(void) {
    init();
    uint8_t ret = FAILURE;

    while(1)
    {
        if (current_game_state == IN_PROGRESS) {
            ret = scan_keypad(&keypad, &cur_char);
            if (ret == SUCCESS) {
                // ignore non-digit input bc i literally do not want to bother with it
                if (cur_char == 'A' || cur_char == 'B' || cur_char == 'C' || cur_char == 'D' ||
                    cur_char == '*' || cur_char == '#') {
                        continue;
                }
                // convert char to enum
                LED_State state;
                switch (cur_char) {
                    case '1':
                        state = RED;
                        break;
                    case '2':
                        state = ORANGE;
                        break;
                    case '3':
                        state = YELLOW;
                        break;
                    case '4':
                        state = GREEN;
                        break;
                    case '5':
                        state = BLUE;
                        break;
                    case '6':
                        state = PURPLE;
                        break;
                    case '7':
                        state = CYAN;
                        break;
                    case '8':
                        state = PINK;
                        break;
                    case '9':
                        state = MAGENTA;
                        break;
                    default:
                        state = OFF;
                }
                // program wants to always set first led then second led
                if (first_led_set == 0) {
                    set_LED(&led1, state, 0);
                    pin[0] = state;
                    first_led_set = 1;
                    __delay_cycles(1000);
                } else {
                    set_LED(&led2, state, 1);
                    pin[1] = state;
                    first_led_set = 0;
                    __delay_cycles(1000);
                }
            }
            pin[2] = get_dipswitch();

        } else if (current_game_state == CHECK){
            // need to check password attempt
            int i;
            uint8_t check = SUCCESS;
            for (i = 0; i < 3; i++){
                if (pin[i] != secret_pin[i]){
                    check = FAILURE;
                }
            }
            // if check success, set current game state to win
            // else need to allow another attempt
            if (check == SUCCESS) {
                current_game_state = WIN;
                update_lcd = 1;
            } else {
                current_game_state = IN_PROGRESS;
            }
        }

        if (update_lcd) {
            update_lcd = 0;
            transmit_to_lcd();
        }

        if (trigger_buzzer) {
            P2IFG |= BIT5;
        }

        __delay_cycles(100000);
    }
}


// -- Interrupt Service Routines ------------------------ //

/**
* transmit_data
*/
#pragma vector = EUSCI_B0_VECTOR
__interrupt void transmit_data(void)
{
    switch(UCB0IV)             // determines which IFG has been triggered
    {
        case USCI_I2C_UCNACKIFG:
            UCB0CTL1 |= UCTXSTT;                      //resend start if NACK
            break;                                    // Vector 4: NACKIFG break
        case USCI_I2C_UCTXIFG0:
            if (tx_byte_cnt > 16)                                // Check TX byte counter
            {
                UCB0TXBUF = lcd_line1_strings[current_game_state][index1];            // Load TX buffer
                index1++;
                tx_byte_cnt--;                              // Decrement TX byte counter
            }
            else if (tx_byte_cnt > 0) {
                UCB0TXBUF = lcd_line2[index2];
                index2++;
                tx_byte_cnt--;
            }
            else
            {
                UCB0CTLW0 |= UCTXSTP;                     // I2C stop condition
                UCB0IFG &= ~UCTXIFG;                      // Clear USCI_B0 TX int flag
            } 
            break;                                    
        default:
            break;
    }

}


/**
* Heartbeat LED, read time every 1s
*/
#pragma vector = TIMER0_B0_VECTOR
__interrupt void heartbeat_LED(void)
{
    P1OUT ^= BIT0;          // LED1 xOR

    if (current_game_state == IN_PROGRESS) {
        // get seconds
        tensecs = seconds / 10;
        secs = (seconds % 10);
        __delay_cycles(100);
        lcd_line2[0] = (tensecs + '0');
        lcd_line2[1] = (secs + '0');
        __delay_cycles(100);
        if (seconds == 0) {
            trigger_buzzer = 1;
            current_game_state = LOSE;
        } else {
            seconds--;
        }
        update_lcd = 1;
    }
    

    TB0CCTL0 &= ~CCIFG;     // clear flag
}
// ----- end heartbeat_LED-----

/*
 * Handles changes in state due to button press
 */
#pragma vector = PORT4_VECTOR
__interrupt void get_button_press(void)
{
    game_state prev_state = current_game_state;
    if (current_game_state == IDLE) {
        current_game_state = IN_PROGRESS;
    } else if (current_game_state == IN_PROGRESS) {
        current_game_state = CHECK;
    } else if (current_game_state == LOSE
                || current_game_state == WIN) {
        // reset game stats, return to idle
        seconds = 30;
        lcd_line2[0] = '0';
        lcd_line2[1] = '0';
        first_led_set = 0;
        set_LED(&led1, OFF, 0);
        set_LED(&led2, OFF, 1);

        int i;
        for (i = 0; i < 3; i++) {
            pin[i] = 0;
        }
        current_game_state = IDLE;
    }

    if (current_game_state != prev_state) {
        update_lcd = 1;
    }

    P4IFG &= ~BIT1;
}
// ----- end get_button_press-----

/*
 * Handles changes in state due to button press
 */
#pragma vector = PORT2_VECTOR
__interrupt void toggle_buzzer(void)
{
    trigger_buzzer = 0;
    P2OUT ^= BIT5;
    __delay_cycles(500000);
    __delay_cycles(500000);
    __delay_cycles(500000);
    P2OUT ^= BIT5;

    P2IFG &= ~BIT5;
}
// ----- end toggle buzzer-----
