Nordic NRF52 LCD driver (1602, 2004 etc) for HD447780 and I2C piggyback.

A problem with the I2C piggyback is timing between data and commands which is normally resolved
with CPU based delays. Well hold on to your socks, because this library executes multiple commands
and data insertions as a transaction with no CPU intervention - just a single interrupt at the end 
(write the whole screen in various places if you want).

A transaction is began as soon as any of the commands are invoked. Because a transaction is flushed at 
the end of the main loop you Mmust include lcdTransactionEnd();` at the end of the main loop 
but before any sleep mode code. 

Usage -

Please configure the top of lcd2004.c and then before your main loop add `lcd2004Init()`.

```
lcdTransactionAppendString("Hello ");
lcdTransactionAppendInt(123); // print up to a 32 bit signed int
lcdSetCursor(0,1);
lcdTransactionAppendString("World ");
lcdTransactionAppendChar('0');
```

Output -

```
HELLO 123
WORLD 0
```

Lots of small writes at random times? Consider one transaction to redraw everything at an interval.

Resources required; 2x timer, 3x PPI channels, 1x TWIM

Please configure the top of lcd2004.c and don't forget to enter `lcdTransactionEnd();` at the 
bottom of your main loop.

PS) If you're using a 5V LCD you're best of using a 2xNMOS for level converting. The inverted 3.3v 
doesn't work on LCD2004 boards. The NRF52DK is actually 3V and it's just not enough to drive the backlight.
