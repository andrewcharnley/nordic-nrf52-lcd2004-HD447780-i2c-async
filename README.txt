 /*
 * Nordic NRF52 LCD driver (1602, 2004 etc) for HD447780 and I2C piggyback.
 *
 *
 * A problem with the I2C piggyback is timing between data and commands which is normally resolved
 * with CPU based delays. Well hold on to your socks, because this library executes multiple commands
 * and data insertions as a transaction with NO CPU overhead!
 *
 * If you're using a 5V LCD you're best of using a 2xNMOS for level converting.
 *
 * Usage -
 *
 * lcdTransactionStart();
 * lcdTransactionAppendString("Hello ");
 * lcdTransactionAppendInt(123); // print up to a 32 bit signed int
 * lcdSetCursor(0,1);
 * lcdTransactionAppendString("World ");
 * lcdTransactionAppendChar('0');
 * lcdTransactionEnd();
 * 
 * Output -
 *
 * HELLO 123
 * WORLD 0
 *
 * Using interrupts? Set flags and create transactions in the main loop.
 * Lots of small writes at random times? Consider one transaction to redraw everything at an interval.
 *
 * Resources required; 2x timer, 3x PPI channels, 1x TWIM
 * If you're low on timers CFG_TIMER_LCD_SPACE can be replaced with an RTC. Adjust CFG_TIMER_LCD_SPACE and hack the reg names to suit.
 *
 * Pleae configure the top of lcd2004.c
 */
