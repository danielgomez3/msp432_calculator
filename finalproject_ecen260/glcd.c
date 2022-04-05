#include "common.h"
#include "character_table.h"
#include "glcd.h"


#define CE  BIT0      // P.6 Chip enable
#define RESET BIT6    // P6.6 reset
#define DC  BIT7     // P6.7 data/control (D/!C)

// Port 4 Pins (Keypad Data Pins)
#define DATAPINS (BIT0|BIT1|BIT2|BIT3)
#define DA BIT0

extern const char character_table[][6];

void GLCD_init(void){
    /*In order to send commands to our screen to initialize it, we have to send it using the communication
     * protocol, which is SPI. We need to initialize SPI*/
    SPI_init();
    /*this is in order to send signals with that protocol. These are the 3 wires
     * that we are using in conjunction with our data wires to send and receive data*/
    P6->DIR |= (CE | RESET | DC); // p6.0, p6.6, p6.7
    P6->OUT |= CE;                  // CE will be idle when high
    P6->OUT &= ~RESET;              // assert reset
    /*Since our reset is  also active-high so putting it low means reset. Once we put it low, we need to put it high again because it is the active going low that resets it:*/
    P6->OUT |= RESET;               // de-assert reset

    /*configure the screen*/
    GLCD_command_write(0x21);   // set extended command mode
    GLCD_command_write(0xB8);   // set LCD Vip for contrast
    GLCD_command_write(0x04);   // set temp coefficient
    GLCD_command_write(0x14);   // set LCD bias mode 1:48
    GLCD_command_write(0x20);   // set normal command mode
    GLCD_command_write(0x0C);    // set display normal mode

}

void GLCD_setCursor(unsigned char x, unsigned char y){
    GLCD_command_write(0x80 | x);   // column
    GLCD_command_write(0x40 | y);   // bank (8 rows per bank)
}

/* clears the GLCD by writing zeros to the entire screen */
void GLCD_clear(void){
  int i;
  for(i = 0; i < (GLCD_WIDTH * GLCD_HEIGHT / 8); i++) {
      GLCD_data_write(0x00);
    }
}

/* write to GLCD controller data register */
void GLCD_data_write(unsigned char data){
    /*if we are sending a command, we want our DCline to be in the 'C' mode, or low (look at datasheet GLCD*/
    P6->OUT |= DC;     //enter "command" mode by inverting bit 7!
    /*now we can actually send the data with the SPI protocol!*/
    SPI_write(data);        //send data via SPI
 }

void GLCD_command_write(unsigned char data){
    /*if we are sending a command, we want our DCline to be in the 'C' mode, or low (look at datasheet GLCD*/
    P6->OUT &= ~DC;     //enter "command" mode by inverting bit 7!
    /*now we can actually send the data with the SPI protocol!*/
    SPI_write(data);        //send data via SPI
}
//
// displays a character, ONLY from font_table. VERY flawed, must replace
void GLCD_putchar(int c){
  int i;
  for(i = 0; i < 6; i++)
    GLCD_data_write(character_table[c][i]);
}

/*Remember, SPI stands for Serial Peripheral Interface*/
void SPI_init(void){
    EUSCI_B0->CTLW0 = 0x0001;   // put UCB0 in reset mode
    EUSCI_B0->CTLW0 = 0x69C1;   // PH=0, PL=1, MSB first, Master, SPI, SMCLK
    EUSCI_B0->BRW = 3;          // 3 MHz / 3 = 1MHz
    EUSCI_B0->CTLW0 &= ~0x001;  // enable UCB0 after config

    P1->SEL0 |= BIT5 | BIT6;
    P1->SEL1 &= ~(BIT5 | BIT6);

    P6->DIR |= (CE | RESET | DC); // P6.7, P6.6, P6.0 set as output
    P6->OUT |= CE;                // CE idle high
    P6->OUT &= ~RESET;            // assert reset
}

void SPI_write(unsigned char data){
    /* to send serial data, last lab we sent it using the Tx buffer!*/
    P6->OUT &= ~CE;               // assert /CE
    EUSCI_B0->TXBUF = data;       // write data
    while(EUSCI_B0->STATW & BIT0); // wait for transmit done
    P6->OUT |= CE;                  // deassert it cuz we are done
}
