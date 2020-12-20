#ifndef _LCD2004_H
#define _LCD2004_H

/*
 * COPYRIGHT FREE.
 * PRODUCED FOR NRF52 by Andrew Charnley, IGAROCOM Ltd, UK.
 * PLEASE SEE lcd2004.c for description and config.
 */

#include "nrf_delay.h"
#include "nrf.h"
#include <stdint.h>
#include <string.h>

// 1KHz is about fastest speed for transfering and actioning a command or data
#define LCD_TICKS_BETWEEN_DATA 15000 // 13k seems lowest, go with that plus a margin
#define LCD_PAYLOAD_CMD_DATA 3

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYSET 0x08
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_FUNCTIONSET 0x02
#define LCD_8BITMODE 0x01
#define LCD_4BITMODE 0x00
#define LCD_4LINE 0x08
#define LCD_2LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define LCD_CMD_EN 0b100  // Enable bit
#define LCD_CMD_RS 0b1  // Register select bit

#define LCD_BUFFER_LENGTH (85U * 6U) // multiple of 6 (for 4 bit mode). Larger buffer gives a few less interrupts on large screen updates
#define LCD_BUFFER_ARRAY 10 // expand this is you're doing a lot of small changes and the lcd can't keep up.

#define LCD_I2C_PIN_INIT_CONF ( (GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) \
  | (GPIO_PIN_CNF_DRIVE_H0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
  | (GPIO_PIN_CNF_PULL_Pullup    << GPIO_PIN_CNF_PULL_Pos)  \
  | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
  | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos))

void lcdTransactionAppendChar (char data);
void lcdTransactionAppendString (char *str);
void lcdTransactionAppendInt (int32_t i);
void lcdTransactionEnd (void);
void lcdTransactionAppendAscii (uint8_t data);
void lcdTransactionSetCursor (uint8_t col, uint8_t row);
void lcdTransactionAddChar (uint8_t location, uint8_t charmap[]);
void lcd2004Init (void);

#endif // _LCD2004_H    // Put this line at the end of your file.