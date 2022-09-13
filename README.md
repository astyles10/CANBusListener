# CANBusListener

Project aims to listen to the CAN bus on a 2012 Hyundai IX35 in order to perform some action (TBD)
Cars use On Board Diagnostics (OBD2) standard protocol, which is built on top of the CAN bus protocol.

Pins of interest are 6 (CAN High) and 14 (CAN LOW), along with ground in 
Probably need 250Kbps data rate when connected
https://en.wikipedia.org/wiki/On-board_diagnostics#OBD-II_diagnostic_connector

Useful Links:
https://www.reddit.com/r/CarHacking/
https://www.seeedstudio.com/blog/2019/11/27/introduction-to-can-bus-and-how-to-use-it-with-arduino/
https://www.instructables.com/Hack-your-vehicle-CAN-BUS-with-Arduino-and-Seeed-C/
https://www.instructables.com/CAN-Bus-Sniffing-and-Broadcasting-with-Arduino/

OBD2 Message Format (Parameter IDs):
https://en.wikipedia.org/wiki/OBD-II_PIDs

Arduino Library for OBD2:
https://www.arduino.cc/reference/en/libraries/obd2/

Equipment List:
https://core-electronics.com.au/obd-ii-to-db9-cable.html
https://wiki.seeedstudio.com/CAN-BUS_Shield_V2.0/

TODO:

With CAN IDs gathered we need to filter unique messages for each ID.
Just store these in a map of IDs to vector of strings
 - Due to RAM limitations we might want to log only for a specific CAN ID, and drop the messages for other IDs.

Assuming the messages run in the same cycle until something happens, collecting messages for a certain amount of time.
With a small log of messages, when some parameter changes a new message should be logged, e.g. a door opening.
