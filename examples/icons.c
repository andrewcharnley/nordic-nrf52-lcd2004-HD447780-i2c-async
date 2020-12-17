
#include "lcd2004.h"

enum {
  LCD_CHAR_CHECK,
  LCD_CHAR_SKULL,
  LCD_CHAR_CROSS
};

static uint8_t check[] = {
  0b00000,
  0b00000,
  0b00001,
  0b00010,
  0b10100,
  0b01000,
  0b00000,
  0b00000
};

static uint8_t skull[] = {
  0b00000,
  0b01110,
  0b10101,
  0b11011,
  0b01110,
  0b01110,
  0b00000,
  0b00000
};

static uint8_t cross[] = {
  0b00000,
  0b10001,
  0b01010,
  0b00100,
  0b01010,
  0b10001,
  0b00000,
  0b00000
};

void lcdInit (void) {

  // this call is blocking
  lcd2004Init();
  
  // async from now on
  lcdTransactionStart();
  lcdTransactionAddChar(LCD_CHAR_CROSS, cross);
  lcdTransactionAddChar(LCD_CHAR_CHECK, check);
  lcdTransactionAddChar(LCD_CHAR_SKULL, skull);
  lcdTransactionSetCursor(0,0);
  lcdTransactionAppendAscii(LCD_CHAR_CHECK);
  lcdTransactionSetCursor(0,1);
  lcdTransactionAppendAscii(LCD_CHAR_CROSS);
  lcdTransactionSetCursor(0,2);
  lcdTransactionAppendAscii(LCD_CHAR_SKULL);
  lcdTransactionEnd();
}
