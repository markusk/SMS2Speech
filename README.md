# SMS2Speech
Receives SMS on an Arduino FONA and converts them to Mac OS Speech

He used hardware consists of 3 parts:
- an Adafruit FONA module
- any Arduino board (a Nano or Uno is okay)
- any Mac OS computer

The software consits of 2 parts:
- the Arduino software which does all the SMS stuff.
- the Mac OS software which is written with the Qt C++ framework.

When the software runs on your Mac, it "pulls" the data from the Arduino in an endless loop via the USB port.
Needed fixes: Due to ASCII restrictions(?) the software is not able to transfer umlauts or other special characters.
