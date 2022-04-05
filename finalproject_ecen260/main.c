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
#include "common.h"
#include "character_table.h"
#include "glcd.h"


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
void display_title();


char calculator_input[3][6]; // store 3 entries of strings, 6 chars long
char error_reported;

enum characters{zero,one,two,three,four,five,six,seven,eight,nine,
  A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z,
  spacebar,period,exclamation,asterisk,hashtag,
  plus,minus,multiply,divide,atsign,lowerj,lowero, lowerd, lowera,lowern,lowerg,colon};

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

    // NOT USED: clear_input_buffer();
    display_title();

    while (1); // wait for an interrupt

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
  for(j = 0; j < result_length - 1; j++){
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
            case ('A'): value_to_send = A; break;
            case ('B'): value_to_send = B; break;
            case ('C'): value_to_send = C; break;
            case ('D'): value_to_send = D; break;
            case ('E'): value_to_send = E; break;
            case ('F'): value_to_send = F; break;
            case ('G'): value_to_send = G; break;
            case ('H'): value_to_send = H; break;
            case ('I'): value_to_send = I; break;
            case ('J'): value_to_send = J; break;
            case ('K'): value_to_send = K; break;
            case ('L'): value_to_send = L; break;
            case ('M'): value_to_send = M; break;
            case ('N'): value_to_send = N; break;
            case ('O'): value_to_send = O; break;
            case ('P'): value_to_send = P; break;
            case ('Q'): value_to_send = Q; break;
            case ('R'): value_to_send = R; break;
            case ('S'): value_to_send = S; break;
            case ('T'): value_to_send = T; break;
            case ('U'): value_to_send = U; break;
            case ('V'): value_to_send = V; break;
            case ('W'): value_to_send = W; break;
            case ('X'): value_to_send = X; break;
            case ('Y'): value_to_send = Y; break;
            case ('Z'): value_to_send = Z; break;

            case (' '): value_to_send = spacebar; break;
            case (0x2E): value_to_send = period; break;
            case ('!'): value_to_send = exclamation; break;
            case ('*'): value_to_send = asterisk; break;
            case ('#'): value_to_send = hashtag; break;

            case ('+'): value_to_send = plus; break;
            case ('-'): value_to_send = minus; break;
            case ('x'): value_to_send = multiply; break;
            case ('%'): value_to_send = divide; break;
            case ('@'): value_to_send = atsign; break;
            case ('j'): value_to_send = lowerj; break;
            case ('o'): value_to_send = lowero; break;
            case ('d'): value_to_send = lowerd; break;
            case ('a'): value_to_send = lowera; break;
            case ('n'): value_to_send = lowern; break;
            case ('g'): value_to_send = lowerg; break;
            case (':'): value_to_send = colon; break;
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

void display_title(){
  char title1[] = "MSP432 -";
  GLCD_setCursor(4,0);
  GLCD_putnumber(title1, sizeof(title1));
  char title2[] = "CALCULATOR";
  GLCD_setCursor(8,1);
  GLCD_putnumber(title2, sizeof(title2));
  char title3[] = "AUTHOR:";
  GLCD_setCursor(4,2);
  GLCD_putnumber(title3, sizeof(title3));

  char author[] = "DANIEL GOMEZ";
    GLCD_setCursor(8,3);
    GLCD_putnumber(author, sizeof(author));

  char handle[] = "@jodango";
  GLCD_setCursor(10,5);
  GLCD_putnumber(handle, sizeof(handle));
  }
