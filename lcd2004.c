#include "lcd2004.h"

/*
 * COPYRIGHT FREE.
 *
 * PRODUCED FOR NRF52 by Andrew Charnley, IGAROCOM Ltd, UK.
 *
 * You need to set the following.
 */

#define CFG_PIN_LCD_SDA 5
#define CFG_PIN_LCD_SCL 6
#define CFG_PIN_LCD_VDD 7 // used for the 2xNMOS level translator gate, or comments out

#define CFG_PPI_LCD_CHANNEL_TWIM_START 2
#define CFG_PPI_LCD_CHANNEL_COUNTER_TICK 3
#define CFG_PPI_LCD_CHANNEL_COUNTER_RESET 4
#define CFG_TIMER_LCD_SPACE NRF_TIMER3
#define CFG_TIMER_LCD_COUNTER NRF_TIMER4
#define CFG_TIMER_LCD_COUNTER_IRQHANDLER TIMER4_IRQHandler
#define CFG_TIMER_LCD_COUNTER_IRQN TIMER4_IRQn

#define CFG_TWIM_LCD_TX NRF_TWIM0

#define LINES LCD_4LINE // choose 2 or 4 line display

/*
 * End of Config
 */

// PRIVATE

struct dmaBufferProto {
  uint16_t size;
  uint8_t data[LCD_BUFFER_LENGTH];
};

static struct {
  struct dmaBufferProto buffers[LCD_BUFFER_ARRAY];
  uint8_t locked;
  uint8_t writing;
  struct dmaBufferProto *writeBuffer;
  struct dmaBufferProto *transferBuffer;
} dmaBuffer;

static struct dmaBufferProto* getNextBuffer (struct dmaBufferProto *buffer) {

  buffer++;
  return (buffer > &dmaBuffer.buffers[LCD_BUFFER_ARRAY-1])? &dmaBuffer.buffers[0] : buffer;
};

static void __startTransfer (void) {

  dmaBuffer.writing = 1;
  CFG_TWIM_LCD_TX->TXD.PTR = (uint32_t) &dmaBuffer.transferBuffer->data[0];
  CFG_TIMER_LCD_COUNTER->CC[0] = dmaBuffer.transferBuffer->size / NRF_TWIM0->TXD.MAXCNT;
  CFG_TIMER_LCD_SPACE->TASKS_START = 1;
}

static void transactionWrite (uint8_t* data, uint8_t size) {

  uint8_t *ref = &dmaBuffer.writeBuffer->data[0];
  ref += dmaBuffer.writeBuffer->size;
  memcpy(ref, data, size);
  dmaBuffer.writeBuffer->size += size;
}

static void send (uint8_t data, uint8_t mode) {

  uint8_t packet[4];
  uint8_t focus = data & 0xF0;
  packet[0] = focus | mode | LCD_BACKLIGHT | LCD_CMD_EN;
  packet[1] = focus | mode | LCD_BACKLIGHT;
  focus = (data << 4) & 0xF0;
  packet[2] = focus | mode | LCD_BACKLIGHT | LCD_CMD_EN;
  packet[3] = focus | mode | LCD_BACKLIGHT;
  transactionWrite(&packet[0], 4);
}

static void transactionAppendCommand (uint8_t data) {

  send(data, 0);
}

// used only on init
static void lcdCmd4 (uint8_t cmd) {

  lcdTransactionStart();
  transactionAppendCommand(cmd);
  lcdTransactionEnd();
  while (dmaBuffer.writing) {}
  nrf_delay_ms(2);
}

// PUBLIC

void lcdTransactionStart (void) {

  dmaBuffer.locked = 1;
  dmaBuffer.writeBuffer = getNextBuffer(dmaBuffer.writeBuffer);
}

void lcdTransactionEnd (void) {
  
  dmaBuffer.locked = 0;
  if (!dmaBuffer.writing) {
    __startTransfer();
  }
}

void lcdTransactionAddChar(uint8_t location, uint8_t charmap[]) {
	
  transactionAppendCommand(LCD_SETCGRAMADDR | (location << 3));
  for (uint8_t i=0; i<8; i++) {
    lcdTransactionAppendAscii(charmap[i]);
  }
}

void lcdTransactionAppendAscii (uint8_t data) {

  send(data, LCD_CMD_RS);
}

void lcdTransactionAppendChar (char data) {

  lcdTransactionAppendAscii((uint8_t) data);
}

void lcdTransactionAppendString (char *str) {

  while (*str) {
    lcdTransactionAppendAscii(*str++);
  }
}

static char *itoa_simple_helper(char *dest, int i) {
  if (i <= -10) {
    dest = itoa_simple_helper(dest, i/10);
  }
  *dest++ = '0' - i%10;
  return dest;
}

void lcdTransactionAppendInt (int32_t i) {

  char dest[11];
  char *s = dest;
  if (i < 0) {
    *s++ = '-';
  } else {
    i = -i;
  }
  *itoa_simple_helper(s, i) = '\0';
  lcdTransactionAppendString(s);
}

void lcdTransactionSetCursor(uint8_t col, uint8_t row) {

  uint8_t row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  transactionAppendCommand(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void lcd2004Init (void) {

  NRF_GPIO->PIN_CNF[CFG_PIN_LCD_SCL] = LCD_I2C_PIN_INIT_CONF;
  NRF_GPIO->PIN_CNF[CFG_PIN_LCD_SDA] = LCD_I2C_PIN_INIT_CONF;
  #ifdef CFG_PIN_LCD_VDD
  NRF_GPIO->PIN_CNF[CFG_PIN_LCD_VDD] = ((GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos) | (GPIO_PIN_CNF_DRIVE_S0S1 << GPIO_PIN_CNF_DRIVE_Pos) | (GPIO_PIN_CNF_INPUT_Disconnect << GPIO_PIN_CNF_INPUT_Pos) | (GPIO_PIN_CNF_DIR_Output << GPIO_PIN_CNF_DIR_Pos));
  NRF_GPIO->OUTSET = 1 << CFG_PIN_LCD_VDD;
  #endif

  CFG_TIMER_LCD_SPACE->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
  CFG_TIMER_LCD_SPACE->PRESCALER = 0;
  CFG_TIMER_LCD_SPACE->CC[0] = 1;
  CFG_TIMER_LCD_SPACE->CC[1] = LCD_TICKS_BETWEEN_DATA - 100U;
  CFG_TIMER_LCD_SPACE->CC[2] = LCD_TICKS_BETWEEN_DATA;
  CFG_TIMER_LCD_SPACE->SHORTS = TIMER_SHORTS_COMPARE2_CLEAR_Enabled << TIMER_SHORTS_COMPARE2_CLEAR_Pos;

  CFG_TIMER_LCD_COUNTER->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
  CFG_TIMER_LCD_COUNTER->PRESCALER = 0;
  CFG_TIMER_LCD_COUNTER->MODE = TIMER_MODE_MODE_LowPowerCounter << TIMER_MODE_MODE_Pos;
  CFG_TIMER_LCD_COUNTER->SHORTS = TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
  CFG_TIMER_LCD_COUNTER->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
 
  CFG_TWIM_LCD_TX->ADDRESS = 0x3f;
  CFG_TWIM_LCD_TX->FREQUENCY = TWIM_FREQUENCY_FREQUENCY_K100 << TWIM_FREQUENCY_FREQUENCY_Pos;
  CFG_TWIM_LCD_TX->PSEL.SCL = (TWIM_PSEL_SCL_CONNECT_Connected << TWIM_PSEL_SCL_CONNECT_Pos) | CFG_PIN_LCD_SCL;
  CFG_TWIM_LCD_TX->PSEL.SDA = (TWIM_PSEL_SDA_CONNECT_Connected << TWIM_PSEL_SDA_CONNECT_Pos) | CFG_PIN_LCD_SDA;
  CFG_TWIM_LCD_TX->SHORTS = TWIM_SHORTS_LASTTX_STOP_Enabled << TWIM_SHORTS_LASTTX_STOP_Pos;
  CFG_TWIM_LCD_TX->TXD.LIST = TWIM_TXD_LIST_LIST_ArrayList << TWIM_TXD_LIST_LIST_Pos;
  CFG_TWIM_LCD_TX->TXD.MAXCNT = LCD_PAYLOAD_CMD_DATA;
  CFG_TWIM_LCD_TX->ENABLE = TWIM_ENABLE_ENABLE_Enabled << TWIM_ENABLE_ENABLE_Pos;

  NRF_PPI->CH[CFG_PPI_LCD_CHANNEL_TWIM_START].EEP = (uint32_t) &CFG_TIMER_LCD_SPACE->EVENTS_COMPARE[0]; // wire timer compare
  NRF_PPI->CH[CFG_PPI_LCD_CHANNEL_TWIM_START].TEP = (uint32_t) &CFG_TWIM_LCD_TX->TASKS_STARTTX; 
  NRF_PPI->CH[CFG_PPI_LCD_CHANNEL_COUNTER_TICK].EEP = (uint32_t) &CFG_TIMER_LCD_SPACE->EVENTS_COMPARE[1]; // wire timer compare
  NRF_PPI->CH[CFG_PPI_LCD_CHANNEL_COUNTER_TICK].TEP = (uint32_t) &CFG_TIMER_LCD_COUNTER->TASKS_COUNT; 
  NRF_PPI->CH[CFG_PPI_LCD_CHANNEL_COUNTER_RESET].EEP = (uint32_t) &CFG_TIMER_LCD_COUNTER->EVENTS_COMPARE[0]; // wire timer compare
  NRF_PPI->CH[CFG_PPI_LCD_CHANNEL_COUNTER_RESET].TEP = (uint32_t) &CFG_TIMER_LCD_SPACE->TASKS_STOP;
  NRF_PPI->FORK[CFG_PPI_LCD_CHANNEL_COUNTER_RESET].TEP = (uint32_t) &CFG_TIMER_LCD_SPACE->TASKS_CLEAR;
  NRF_PPI->CHENSET = 1 << CFG_PPI_LCD_CHANNEL_TWIM_START | 1 << CFG_PPI_LCD_CHANNEL_COUNTER_TICK | 1 << CFG_PPI_LCD_CHANNEL_COUNTER_RESET;  // enable ppi channel

  NVIC_EnableIRQ(CFG_TIMER_LCD_COUNTER_IRQN);

  dmaBuffer.transferBuffer = &dmaBuffer.buffers[0];
  dmaBuffer.writeBuffer = &dmaBuffer.buffers[LCD_BUFFER_ARRAY-1];

  // 8 bit mode
  nrf_delay_ms(200); // wait for >40ms
  lcdCmd4(LCD_FUNCTIONSET|LCD_8BITMODE);
  nrf_delay_ms(5); // wait for >4.1ms
  lcdCmd4(LCD_FUNCTIONSET|LCD_8BITMODE);
  nrf_delay_ms(5); // wait for >4.1ms
  lcdCmd4(LCD_FUNCTIONSET|LCD_8BITMODE);
  // 4 bit mode
  lcdCmd4(LCD_FUNCTIONSET|LCD_4BITMODE);
  lcdCmd4(LCD_FUNCTIONSET | LCD_4BITMODE | LINES);
  lcdCmd4(LCD_DISPLAYSET | LCD_DISPLAYOFF | LCD_CURSOROFF | LCD_BLINKOFF);
  lcdCmd4(LCD_CLEARDISPLAY);
  nrf_delay_ms(2000);
  lcdCmd4(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT);
  //lcdCmd4(LCD_RETURNHOME);
  //nrf_delay_ms(2000);
  lcdCmd4(LCD_SETDDRAMADDR); // fast home
  lcdCmd4(LCD_DISPLAYSET | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF);
  nrf_delay_ms(1);
}

// INTERRUPTS

void CFG_TIMER_LCD_COUNTER_IRQHANDLER (void) {

  CFG_TIMER_LCD_COUNTER->EVENTS_COMPARE[0] = 0;
  dmaBuffer.transferBuffer->size = 0;
  dmaBuffer.transferBuffer = getNextBuffer(dmaBuffer.transferBuffer);
  dmaBuffer.writing = 0;
  if (!dmaBuffer.locked && dmaBuffer.transferBuffer->size) {
    __startTransfer();
  }
}
