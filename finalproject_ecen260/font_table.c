#include "font_table.h"

//NOTE: In the future for optimization, consider making multiple char arrays (e.g 'Numbers', 'special_characters')

const char character_table[][6]= {
                             {0x3e,0x51,0x49,0x45,0x3e,0x00}, /* number 0 */
                             {0x44,0x42,0x7F,0x40,0x40,0x00}, /* number 1 */
                             {0x42,0x61,0x51,0x49,0x46,0x00},/* number 2 */
                             {0x22,0x41,0x49,0x49,0x36,0x00},/* number 3 */
                             {0x0C,0x0A,0x09,0x7F,0x08,0x00},/* number 4 */
                             {0x27,0x49,0x49,0x49,0x31,0x00},/* number 5 */
                             {0x3E,0x49,0x49,0x49,0x32,0x00},/* number 6 */
                             {0x01,0x61,0x19,0x05,0x03,0x00},/* number 7 */
                             {0x36,0x49,0x49,0x49,0x36,0x00},/* number 8 */
                             {0x26,0x49,0x49,0x49,0x3E,0x00},/* number 9 */
                             {0x08,0x08,0x3E,0x08,0x08,0x00},  // A (now updated to look like '+'
                             {0x7f, 0x49, 0x49, 0x49, 0x36, 0x00},  // B
                             {0x3e, 0x41, 0x41, 0x41, 0x22, 0x00},  // C
                             {0x7f, 0x41, 0x41, 0x41, 0x3e, 0x00},  /* D fix this top of D up one bit */
                             {0x7f, 0x49, 0x49, 0x49, 0x49, 0x00},  /* E */
                             {0x7f, 0x48, 0x48, 0x48, 0x48, 0x00},  /* F */
                             {0x3e, 0x41, 0x51, 0x51, 0x72, 0x00},  /* G */
                             {0x7f, 0x08, 0x08, 0x08, 0x7f, 0x00},  /* H */
                             {0x41, 0x41, 0x7f, 0x41, 0x41, 0x00},  /* I */
                             {0x42, 0x41, 0x7E, 0x40, 0x40, 0x00},  /* J */
                             {0x7f, 0x18, 0x24, 0x42, 0x01, 0x00},  /* K */
                             {0x7f, 0x40, 0x40, 0x40, 0x40, 0x00},  /* L */
                             {0x7e, 0x01, 0x1e, 0x01, 0x7e, 0x00},  /* M */
                             {0x7f, 0x02, 0x0c, 0x10, 0x7f, 0x00},  /* N */
                             {0x3e, 0x41, 0X41, 0X41, 0X3e, 0x00},  /* O */
                             {0x7f, 0x48, 0x48, 0x48, 0x30, 0x00},  /* P */
                             {0x3c, 0x42, 0X42, 0X43, 0X3d, 0x00},  /* Q not finished */
                             {0x7f, 0x09, 0x19, 0x29, 0x46, 0x00},  /* R */
                             {0x22, 0x51, 0x49, 0x45, 0x22, 0x00},  /* S */
                             {0x01, 0x01, 0x7f, 0x01, 0x01, 0x00},  /* T */
                             {0x3f, 0x40, 0x40, 0x40, 0x3f, 0x00},  /* U */
                             {0x03, 0x1c, 0x60, 0x1c, 0x03, 0x00},  /* V */
                             {0x3f, 0x40, 0x38, 0x40, 0x3f, 0x00},  /* W */
                             {0x11, 0x0a, 0x04, 0x0a, 0x11, 0x00},  /* x */
                             {0x01, 0x02, 0x7c, 0x02, 0x01, 0x00},  /* Y */
                             {0x43, 0x45, 0x49, 0x51, 0x61, 0x00},  /* Z */
                             {0x00, 0x00, 0x00, 0x00, 0x00, 0x00},  // 'space'
                             {0x00, 0x60, 0x60, 0x60, 0x00,},  /* . */
                             {0x00, 0x6f, 0x6f, 0x6f, 0x00,},  /* ! */
                             {0x0A,0x04,0x1F,0x04,0x0A,0x00},/* '*' */
                             {0x12,0x3F,0x12,0x3F,0x12,0x00}/* # */
  };

// const char number_table[][6] = {
//                              {0x3e,0x51,0x49,0x45,0x3e,0x00}, /* number 0 */
//                              {0x44,0x42,0x7F,0x40,0x40,0x00}, /* number 1 */
//                              {0x42,0x61,0x51,0x49,0x46,0x00},/* number 2 */
//                              {0x22,0x41,0x49,0x49,0x36,0x00},/* number 3 */
//                              {0x0C,0x0A,0x09,0x7F,0x08,0x00},/* number 4 */
//                              {0x27,0x49,0x49,0x49,0x31,0x00},/* number 5 */
//                              {0x3E,0x49,0x49,0x49,0x32,0x00},/* number 6 */
//                              {0x01,0x61,0x19,0x05,0x03,0x00},/* number 7 */
//                              {0x36,0x49,0x49,0x49,0x36,0x00},/* number 8 */
//                              {0x26,0x49,0x49,0x49,0x3E,0x00},/* number 9 */
//                              {0x00, 0x60, 0x60, 0x60, 0x00}  /* . */
//};
