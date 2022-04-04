// TODO: Make sprites and individual indexes to the font table for minus, mult, divide, period
// TODO: Look at backend and see what period value is and adjust 'period' enum index to meet it or set it manually
// TODO: REMEMBER: YOU ARE NOT GOING TO HAVE VISUAL HISTORY! once computation is complete,
// the user must clear the screen with hashtag in order to use again
// //TODO: Separate GLCD functions to a different file

/* Calculator Button Notes:
 * The number buttons keypad (0-9) should print those numbers on the LCD.
 * 'A' button (value 0xA) will add
 * 'B' button (Value 0xB) will subtract
 * 'C' button will multiply
 * 'D' button will divide
 * Star/asterisk (value 0xE) will be delete
 * Hashtag (value 0xF) will be enter/compute
 */

#include <stdio.h> // for snprintf()
#include <stdlib.h> // for atio()
#include <string.h>
#include "msp.h"
#include "font_table.h"

#define CE  BIT0      // P.6 Chip enable
#define RESET BIT6    // P6.6 reset
#define DC  BIT7     // P6.7 data/control (D/!C)

// Our screen (graphic LCD) width etc.
#define GLCD_WIDTH  84
#define GLCD_HEIGHT 48


// Port 4 Pins (Keypad Data Pins)
#define DATAPINS (BIT0|BIT1|BIT2|BIT3)
#define DA BIT0

// function prototypes:
void GLCD_setCursor(unsigned char x, unsigned char y);
void GLCD_clear(void);
void GLCD_init(void);
void GLCD_data_write(unsigned char data);
void GLCD_command_write(unsigned char data);
void GLCD_putchar(int c);
void GLCD_putnumber(char computation[], int index);
void SPI_init(void);
void SPI_write(unsigned char data);
//this is for the keypad:
uint8_t keypad_decode();

void evaluate_key();
void evaluate_operation();
void put_in_buffer(int index, char given_value);
void clear_input_buffer();
void clear_line_at_cursor();
void display_error();
void display_computation();
void align_cursor();


char calculator_input[3][6]; // store 3 entries of strings, 6 chars long
extern char character_table[][6];
//char number_table[][6];
char error_reported;

enum characters{zero,one,two,three,four,five,six,seven,eight,nine,
  A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
  spacebar,period,exclamation,asterisk,hashtag,plus,minus,multiply,divide};

  /*TODO: DON'T forget to find out what hex number '.' (period) actually represents in ascii
    and manually set this period enum to that hex value by moving that enum
    to the right spot so they are the same thing*/



/**
 * main.c
 */
void main(void){
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;     // stop watchdog timer

    GLCD_init();    /*initialize our screen (GLCD)*/
    GLCD_clear();
    GLCD_setCursor(0,0);    // return to the home position



    // Configure Data Input Pins (P4.0, 4.1, 4.2, 4.3)
    P4->DIR &= ~DATAPINS;      // set up data pins as inputs
    // Configure "Data Available" Signal Pin (P3.0)
    P3->DIR &= ~DA;       // set up DA pin as input
    P3->REN |=  DA;       // connect pull resistor to DA
    P3->OUT |=  DA;       // configure resistor as pull-up. It is active low!
    P3->IFG &= ~DA;       // clear interrupt flag for DA
    P3->IE |=  DA;       // enable pin-level interrupt for DA
    // Enable Port-level Interrupts (Table 6.2)
    NVIC->ISER[1] |= 0x00000020;  // enable Port 3 interrupts
    // Enable Interrupts Generally
    _enable_interrupts();

    clear_input_buffer();

    while (1); // wait for an interrupt


}

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



/***
* keypad decoder function
***/
uint8_t keypad_decode(){
 uint8_t key = 0;
 /* A local variable 'key' is set to 0. Since the pins in Port 4 have been assigned as our input,
  * We will compare those values against the ones assigned with the global variable 'DATAPINS'.
  * Regardless if there is a match, we save the result to the temporary 'Key' variable. */
 uint8_t data = P4->IN & DATAPINS;
 /* Using a 'Switch' statement, and using our 'data' variable as the logic case expression,
  * we then proved all of our possible outcomes in CASE our 'data' variable has a certain output.
  * For example, if our 'data' variable equals 0x0D (Binary 1101, which it should be knowing its position
  * on the numberpad), our function should return an integer '0' to indicate that the numberpad has just
  * recived a zero from the user. All the other cases are presented in the statement as well!*/
 switch(data){
 case 0x0D: key = 0x0; break;  /* 0 */
   case 0x00: key = 0x1; break; /* 1 */
   case 0x01: key = 0x2; break; /* 2 */
   case 0x02: key = 0x3; break; /* 3 */
   case 0x04: key = 0x4; break; /* 4 */
   case 0x05: key = 0x5; break; /* 5 */
   case 0x06: key = 0x6; break; /* 6 */
   case 0x08: key = 0x7; break; /* 7 */
   case 0x09: key = 0x8; break; /* 8 */
   case 0x0A: key = 0x9; break; /* 9 */
   case 0x03: key = plus; break; /* A */  // Don't set they value to 0xA anymore, make a 'plus'
   case 0x07: key = minus; break; /* B */
   case 0x0B: key = multiply; break; /* C */
   case 0x0F: key = divide; break; /* D */
   case 0x0C: key = period; break; /* This is supposed to put a decimal point '.' down*/
   case 0x0E: key = hashtag; break; /* # */
 }
 return key;
}

/*TODO: Add new calculator functionality. Floating point arithemtic
 Star should be a decimal point.
 'clearing' of the buffer screen to erase mistakes needs to be moved to '#'
The way we are able to press 'enter' or perform a computation, any operation will follow an operation at
any given time.*/
void evaluate_key(int key){
  extern char calculator_input[][];
  extern char error_reported;
  int operation_pressed = 0; //
  char value_put_in_buffer = '\0';
  switch (key){
    case (zero): value_put_in_buffer = '0'; break;
    case (one): value_put_in_buffer = '1'; break;
    case (two): value_put_in_buffer = '2'; break;
    case (three): value_put_in_buffer = '3'; break;
    case (four): value_put_in_buffer = '4'; break;
    case (five): value_put_in_buffer = '5'; break;
    case (six): value_put_in_buffer = '6'; break;
    case (seven): value_put_in_buffer = '7'; break;
    case (eight): value_put_in_buffer = '8'; break;
    case (nine): value_put_in_buffer = '9'; break;
    case (period): value_put_in_buffer = '.';// put '.' in buffer if asterisk ('*') is pressed
    case (hashtag): clear_line_at_cursor(); GLCD_setCursor(0,0); clear_input_buffer(); break; // clear input if '#' is pressed
    default:
        operation_pressed = 1;
        switch (key){
            case (plus):
            value_put_in_buffer = '+';
            break;

            case (minus):
            value_put_in_buffer = '-';
            break;

            case (divide):
            value_put_in_buffer = 'x';// this is multiply
            break;

            case (multiply):
            value_put_in_buffer = '/';// this is divide
            break;
        }
        /* if an operation is pressed but and an opp. is not loaded yet: put her in buffer!*/
        if (operation_pressed && calculator_input[2][0] == NULL){
          calculator_input[2][0] = value_put_in_buffer; GLCD_putchar(key);
        }
        // If an operation is pressed and there already is one in buffer: perform computation!
        else if (operation_pressed && calculator_input[2][0] != NULL)
          evaluate_operation(); return; // make sure to 'return'/leave the function so we don't print the extra operation!
    }
  /*NOTE: The following is the next step for the numerical values, operations have been taken care of
   Let's break this down:
  Our calculator can only do 2 opperand math. The only way we know which operrand is which is
  through our operation sign. If there's an operation sign, present at any point, it should ALWAYS
  be stored as second opp. . We'll error handle later!*/

  /* if there's sign already present, save as first opp.*/
  if (calculator_input[2][0] == NULL)
    put_in_buffer(0, value_put_in_buffer);
  /* Else if there is an operation loaded in, load it into into second row */
  else
    put_in_buffer(1, value_put_in_buffer);

  /* no matter what happens, regardless if our operation goes into buffer, print it out and
   act like we care. We can only do a single operation so we will throw an error later*/
  GLCD_putchar(key);

}

void PORT3_IRQHandler(void){
 uint8_t key = 0;
 if(P3->IFG & DA){ // if there is an interrupt and data available:
    // Clear interrupt flag for DA
    P3->IFG &= ~DA;
    // Read data
    key = keypad_decode();  // determine which key was pressed
    evaluate_key(key);
 }
 //TODO: If the value are any of the operations (mult, add, div), Set off a flag and send the numbers into the proper
 //function for mult, div, etc.
 //put_in_buffer(key);
}

/*This will assign a value to a part of a given row. The first operand of an operation needs to be assigned
  * the first row, the operation to the third row, and the second number they put in to the 2nd row*/
void put_in_buffer(int index, char given_value){
  int i;
  char sentinel = '\0';
  extern char calculator_input[][];
  for (i = 0; i < 6; i++)
    if (calculator_input[index][i] == sentinel){
      calculator_input[index][i] = given_value;
      return;
    }
    else
      continue;
}

void evaluate_operation(){
  extern char calculator_input[][];
  extern char error_reported;
  char result_into_char[17];

  //TODO: Fix error Handling conditions by maybe implementing switch
  if (error_reported != 0) {
    display_error();
    return;
    }
  if (calculator_input[0][0] == NULL || calculator_input[1][0] == NULL || calculator_input[2][0] == NULL){
    display_error();
    return;
    }

  //TODO: Convert charr arrays to values just for math, not needed for anything else!
  double first_opperand = atoi(calculator_input[0]);
  double second_opperand = atoi(calculator_input[1]);
  double result;
  //TODO: Calculate
  switch (calculator_input[2][0]){
    case ('+'): result = first_opperand + second_opperand; break;
    case ('-'): result = first_opperand - second_opperand; break;
    case ('x'): result = first_opperand * second_opperand; break;
    case ('%'): result = first_opperand / second_opperand; break;
    }
  //TODO: after we compute our value, we need to put our cursor the next line over and then print value
  align_cursor();
  //TODO: print out result
  snprintf(result_into_char, 16, "%f",result);
  GLCD_putnumber(result_into_char, sizeof(result_into_char));
  clear_input_buffer();


}

void align_cursor(){
  ;
  }

void clear_input_buffer(){
    extern char calculator_input[][];
    memset(calculator_input, 0, sizeof(calculator_input[0][0]) * 6 * 3); // * 6 * 3 mean rows and cols of 2-D array
    //calculator_input[3][6] == { NULL };
    }

/* This needs to define what our array may be saying, and interact with GLCD_putchar*/
void GLCD_putnumber(char result[], int result_length){
  //TODO: Don't worry about what GLCD_putchar is doing, all we need to remember to do is:
  //pass one integer value to GLCD_putchar for it to print, it will take care of the rest
  int j;
  int current_value = 0;
  int value_to_send = 0;
  for(j = 0; j < 4; j++){
      if (result[current_value] != 0x00){
          switch (result[current_value]){
            case (0x30): value_to_send = zero; break;
            case ('1'): value_to_send = one; break;
            case ('2'): value_to_send = two; break;
            case ('3'): value_to_send = three; break;
            case ('4'): value_to_send = four; break;
            case ('5'): value_to_send = five; break;
            case ('6'): value_to_send = six; break;
            case ('7'): value_to_send = seven; break;
            case ('8'): value_to_send = eight; break;
            case ('9'): value_to_send = nine; break;
            case (0x2E): value_to_send = period; break;
            case ('-'): value_to_send = minus; break;
      }
  }
     GLCD_putchar(value_to_send); // only if the value is not null we can print it!
     /*pointer arithmetic! This make sure that the pointer will point to next element in array that was
     passed into this function. incrementing result[j] won't cut it:*/
     result++;

  }


  }


void display_error(){
  GLCD_putchar(E);
  GLCD_putchar(R);
  GLCD_putchar(R);
  GLCD_putchar(O);
  GLCD_putchar(R);
  GLCD_putchar(exclamation);
  error_reported = 0;
  return;
  }

void clear_line_at_cursor(){
  ;
  }
