Nordic NRF52 LCD driver (1602, 2004 etc) for HD447780 and I2C piggyback.

A problem with the I2C piggyback is timing between data and commands which is normally resolved
with CPU based delays. Well hold on to your socks, because this library executes multiple commands
and data insertions as a transaction with no CPU intervention - just a single interrupt at the end 
(write the whole screen in various places if you want).

A transaction is created as soon as any of the commands are invoked. A transaction by including
lcdTransactionEnd();` either manually at the end of the main loop (before any sleep mode code).

Usage -

Consider adding lcdTransactionEnd(); to the end of your main loop.

Please configure the top of lcd2004.c which references the NRF52 resources you wish to use
and then before your main loop add `lcd2004Init()`.

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

This library uses an array of buffers. One character or command is 6 bytes. If you were to set
cursor (one command) and write a full 4 line of 20 characters that would be 486 bytes. You can set
the buffer array length and size in the header file. Should a transaction exceed the length it will
automatically trigger transmission and move to the next buffer - at the expense of an extra 
interrupt call.

Basically, if you're doing large transactions then use a large length. If you're lots of small
transactions rather than a 'screen refresh' then increase the array length. Remember the LCD is slow
so small transactions aren't ideal. You're better off using a timer to apply updates every 1/2 second
or so. Also if the array overflows then you will see screen corruption. Yep, large transactions are
just plain better.

Resources required; 2x timer, 3x PPI channels, 1x TWIM

Please configure the top of lcd2004.c and don't forget to enter `lcdTransactionEnd();` at the 
bottom of your main loop.

PS) If you're using a 5V LCD you're best of using a 2xNMOS for level converting. The inverted 3.3v 
doesn't work on LCD2004 boards. The NRF52DK is actually 3V and it's just not enough to drive the backlight.
