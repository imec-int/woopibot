#include <Servo.h>

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

Servo servoL;                                // Define the left and right servos
Servo servoR;

int followlineSpeed = 100;

// Perform these steps with the Arduino is first powered on
void setup()
{
  Serial.begin(9600);                        // Set up Arduino Serial Monitor at 9600 baud
  servoL.attach(13);                         // Attach (programmatically connect) servos to pins on Arduino
  servoR.attach(12);
  
  Serial.println("WOOPI NFC ROBOT Reader");
  nfc.begin();
}

// This code repeats indefinitely
void loop() {
  
  // doe iets als hij een NFC tag ziet:
  if (nfc.tagPresent(10)) {
    Serial.println("Found tag");
    NfcTag tag = nfc.read();
    tag.print();


    turn90();
  }else{
    followLine();
//    delay(50); // Delay for 50 milliseconds (1/20 second)
    delay(40); // 50 -10 van de nfc timeout
  }
}

void followLine() {
  int pins = scanLineSensors();
  printLineSensors(pins);
  
  // default waarden = ga gewoon vooruit:
  int vL = followlineSpeed;
  int vR = followlineSpeed;
  
  // based on line sensors:
  switch(pins) {
    case B1000:                        
      vL = -followlineSpeed;
      vR = followlineSpeed;
      break;
    case B1100:
      vL = 0;
      vR = followlineSpeed;
      break;
    case B0100:
      vL = followlineSpeed/2;
      vR = followlineSpeed;
      break;
    case B0110:
      vL = followlineSpeed;
      vR = followlineSpeed;
      break;
    case B0010:
      vL = followlineSpeed;
      vR = followlineSpeed/2;
      break;
    case B0011:
      vL = followlineSpeed;
      vR = 0;
      break;
    case B0001:
      vL = followlineSpeed;
      vR = -followlineSpeed;
      break;
    case B1111: // alles zwart                  
      vL = followlineSpeed;
      vR = followlineSpeed;
      break;
    case B0000: // alles wit                 
      vL = 0;
      vR = 0;
      break;
  }
  servoL.writeMicroseconds(1500 + vL);
  servoR.writeMicroseconds(1500 - vR);
}

int scanLineSensors() {
  DDRD |= B11110000;                         // Set direction of Arduino pins D4-D7 as OUTPUT
  PORTD |= B11110000;                        // Set level of Arduino pins D4-D7 to HIGH
  delayMicroseconds(230);                    // Short delay to allow capacitor charge in QTI module
  DDRD &= B00001111;                         // Set direction of pins D4-D7 as INPUT
  PORTD &= B00001111;                        // Set level of pins D4-D7 to LOW
  delayMicroseconds(230);                    // Short delay
  int pins = PIND;                           // Get values of pins D0-D7
  pins >>= 4;                                // Drop off first four bits of the port; keep only pins D4-D7
  
  return pins;
}

void printLineSensors(int pins) {
  // pad with zeros:
  if (pins < 2) Serial.print(B0);
  if (pins < 4) Serial.print(B0);
  if (pins < 8) Serial.print(B0);
  Serial.println(pins, BIN);
}

void turn180() {
  int turnspeed = 50;  // natte vinger
  int turntime = 2200; // natte vinger
  
  servoL.writeMicroseconds(1500 + turnspeed);
  servoR.writeMicroseconds(1500 + turnspeed);
  delay(turntime);
  servoL.writeMicroseconds(1500);
  servoR.writeMicroseconds(1500);
}


void turn90() {
  int turnspeed = 50;  // natte vinger
  int turntime = 1100; // natte vinger
  
  servoL.writeMicroseconds(1500 + turnspeed);
  servoR.writeMicroseconds(1500 + turnspeed);
  delay(turntime);
  servoL.writeMicroseconds(1500);
  servoR.writeMicroseconds(1500);
}

