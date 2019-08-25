# ESP32-CAN-DRIVER

## CAN-DRIVER v1.0

### Initial Release - ESP32 CAN REGISTER DEFINITION

**test-ESP32CANSettings-on-desktop** - CAN Bit Timing Calculator test on desktop compiler (This can be tested on any C++ compiler) 

src/ESP32CANRegisters - Defines the ESP32 CAN Registers Address.
src/ESP32ACANSettings.h - CAN Bit time calculator and Settings class
src/ESP32ACANSettings.cpp - CAN Bit time calculator and Settings class

##### examples : ARDUINO FILES

examples/ESP32CANBitTimingSettings - Check the bit timing for desired bit rate
examples/ESP32CANRegisterTest - Checks the CAN register access 


## CAN-Driver v1.1

src/ESP32ACAN.h - driver functions
src/ESP32ACAN.cpp
src/CANMessage.h - CAN Message format properties

Self testing the ESP32 CAN Controller. Handling both extended and standard frame formats. The sequence of message controlled by Polling method (using Buffer Status Flags). The CAN Controller reponds well for higher CAN bit rates (250 kbit/s and above), but fails for Slow bit rates (125 kbit/s and below). The loss of frames can be seen with the Intensive Check.

examples : ARDUINO FILES

examples/LoopBackCheck-Polling
examples/LoopBackCheck-IntensivePolling - Sends 10000 messages


## CAN-Driver v1.2

src/ACANBuffer16.h

Message control method by handling Interrupts. Driver buffers are added. The size of the Driver buffers can be changed in src/ESP32ACANSettings.h. The Driver works in NormalMode operation. 

NormalMode Test with - MCP2515 and MCP2517

examples : ARDUINO FILES

examples/LoopBackCheck-Interrupt
examples/LoopBackCheck-IntensiveInterrupt - Sends 10000 messages
examples/ESP32CANTestWith-ACAN2515 - test with MCP2515 Normal Mode operation

## CAN-Driver v2.0

src/ESP32AcceptanceFilters.h

Driver work with reception filters. Single and Dual Filter settings for Standard and Extended Frame can be set. The only message ID defined are accepted and handled by the receiver.

examples : ARDUINO FILES

example/ESP32CANFilterSettings - works with filter settings.



