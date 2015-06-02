# NFC version
First version of Woopibot.
Uses a NFC reader to read out NFC IDs and act on them

## Installation of NFC library

Install the NFC library: https://github.com/elechouse/PN532

# BLE version

Consists of 2 projects: the Arduino on the Woopibot and the RFDuino connected via the Serial to the Arduino.

## woopibotArduino
Works together with woopibotRFDuino. Runs on the Arduino attached to the servos and the sensors of the bot.

Has different functions such as:
- follow line
- read 4 sensors and send them over Serial to woopibotRFDuino
- read ping sensor and send it over Serial to woopibotRFDuino
- accept commands from Serial to drive Woopibot manually
- accept commands to enable/disable follow line function
- accept commands to enable/disable ping sensor read out

## woopibotRFDuino
Works together with woopibotArduino

- Forwards BLE messages to Serial 
- Forwards Serial messages to BLE

## messages
Messages are strings that start with a 0x02 byte and end with a 0x03 byte.

Examples of messages that can be send to the BLE:
- L1500: send 1500 microseconds to the left servo
- R2000: send 2000 microseconds to the right servo
- F1/F0: enable/disable "follow line"
- P1/P0: enable/disable "ping sensor"

Examples of messages that can be received from the BLE:
- S0000: all line sensors see white
- S1000: line sensor 0 sees black
- P230: it takes 230 microseconds for the "ping" to return to the "ping sensor"
- P15900: it takes 15900 microseconds for the "ping" to return to the "ping sensor"






