/**
* @file
* @brief LCD interface functionality
*
*/
#include "src/lcd.h"
#include "intrinsics.h"
#include "msp430fr2310.h"


uint8_t state_flag;         // 0 = just update temp, 1 = pattern, 2 = window
uint8_t current_n;
uint8_t change_allowed;   // flag to tell if the incoming data is allowed to change window size or n

void init_lcd(){
    P1DIR |= BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7;       // EN, RS, DB4, DB5, DB6, DB7
    P1OUT &= ~(BIT0 | BIT1 | BIT4 | BIT5 | BIT6 | BIT7);    // Clear outputs

    state_flag = 0;
    change_allowed = 0;

    PM5CTL0 &= ~LOCKLPM5;
    
    __delay_cycles(50000);
    lcd_set_function();
    DELAY_001;
    // load_custom_chars();
    // DELAY_001;
    lcd_send_command(LCD_DISPLAY_ON);
    DELAY_001;
    lcd_send_command(LCD_ENTRY_MODE_SET);
    DELAY_001;
    lcd_send_command(LCD_CURSOR_ON);
    DELAY_001;
    lcd_send_command(LCD_CURSOR_RIGHT);
    DELAY_001;
    lcd_send_command(LCD_CLEAR_DISPLAY);

}

void lcd_send(uint8_t data, uint8_t is_data){
    if (is_data)
        P1OUT |= BIT1;        // RS = 1 for data
    else
        P1OUT &= ~BIT1;       // RS = 0 for command

    P1OUT = (data & 0xF0);    // Send high nibble (on P1.4-7)
    P1OUT |= BIT0;            // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    DELAY_0001;
    P1OUT = ((data << 4) & 0xF0);
    if (is_data) P1OUT |= BIT1;   // Re-set RS again if needed (optional)
    P1OUT |= BIT0;
    DELAY_0001;
    P1OUT &= ~BIT0;
}

void lcd_send_string(char *str){
    while (*str != '\0'){
        lcd_send_data(*str++);
    }
}

// TODO this function might be repurposed
// void send_lcd_mode(uint8_t mode)
// {
//     lcd_send_command(LCD_RETURN_HOME);
//     DELAY_0001;
//     lcd_send_string((char*)lcd_strings[mode]);
// }

// void lcd_set_time(uint8_t *data)
// {
//     // set DDRAM to bottom left corner
//     lcd_send_command(LCD_BOTTOM_LINE);       
//     DELAY_0001;
//     time_n[2] = data[0] + '0'; // 100 s
//     time_n[3] = data[1] + '0'; // 10 s
//     time_n[4] = data[2] + '0'; // 1 s
//     lcd_send_string(time_n);
// }

void lcd_set_function(){
    P1OUT &= ~BIT1;                 // RS = 0 for command
    P1OUT = (0x03 << 4);            // Send high nibble
    P1OUT |= BIT0;                  // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    __delay_cycles(30000);

    P1OUT = (0x03 << 4);            // Send high nibble
    P1OUT |= BIT0;                  // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    __delay_cycles(30000);

    P1OUT = (0x03 << 4);          // Send high nibble
    P1OUT |= BIT0;                  // Enable pulse
    DELAY_0001;
    P1OUT &= ~BIT0;

    __delay_cycles(30000);

    P1OUT = (0x02 << 4);    // Send low nibble
    P1OUT |= BIT0;
    DELAY_0001;
    P1OUT &= ~BIT0;

    lcd_send_command(LCD_FUNCTION);
}

void lcd_clear_line(uint8_t cmd)
{
    lcd_send_command(cmd);
    DELAY_0001;
    uint8_t i = 0;
    for (i = 0; i < 16; i++) {
        lcd_send_data(' ');
        DELAY_0001;
    }
    lcd_send_command(cmd);
}

void load_custom_chars(){
    uint8_t i;
    lcd_send_command(0x40);     // set CGRAM to address 00
    // for (i = 0; i < 8; i++) {
    //     lcd_send_data(ambient_custom[i]);
    // }

    lcd_send_command(0x41);     // set CGRAM to address 01
    // for (i = 0; i < 8; i++) {
    //     lcd_send_data(plant_custom[i]);
    // }

    lcd_send_command(LCD_RETURN_HOME); // return DDRAM to address 00
}
