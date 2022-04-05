#ifndef _GLCD_H
#define _GLCD_H



// Our screen (graphic LCD) width etc.
#define GLCD_WIDTH  84
#define GLCD_HEIGHT 48

#define CE  BIT0      // P.6 Chip enable
#define RESET BIT6    // P6.6 reset
#define DC  BIT7     // P6.7 data/control (D/!C)

// Port 4 Pins (Keypad Data Pins)
#define DATAPINS (BIT0|BIT1|BIT2|BIT3)
#define DA BIT0

void GLCD_setCursor(unsigned char x, unsigned char y);
void GLCD_clear(void);
void GLCD_init(void);
void GLCD_data_write(unsigned char data);
void GLCD_command_write(unsigned char data);
void GLCD_putchar(int c);
void GLCD_putnumber(char computation[], int index);


// TODO: determine if you can delete these
void SPI_init(void);
void SPI_write(unsigned char data);

#endif
