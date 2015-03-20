#include <Servo.h>

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// Servo stuff:
Servo servoL;                                
Servo servoR;

// Other stuff:
int defaultSpeed = 100;

void setup() {
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  
  
  Serial.begin(9600);                        // Set up Arduino Serial Monitor at 9600 baud
  servoL.attach(13);                         // Attach (programmatically connect) servos to pins on Arduino
  servoR.attach(12);
  
  Serial.println("WOOPI NFC ROBOT Reader");
  nfc.begin();
}

void loop() {
  
  // doe iets als hij een NFC tag ziet:
  if (nfc.tagPresent(10)) {
    Serial.println("Found tag");
    NfcTag tag = nfc.read();
    tag.print();

    // TODO: hier nog checken welke kaart het precies is, voorlopig veronderstellen we dat het altijd een kruispunt is
    // zie https://github.com/mixbe/NFC-2-BLE-on-RFDuino/blob/master/rfduino-readtag/rfduino-readtag.ino voor code om tagid's te vergelijken
    on4wayJunction();
    
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
  int vL = defaultSpeed;
  int vR = defaultSpeed;
  
  // based on line sensors:
  switch(pins) {
    case B1000:                        
      vL = -defaultSpeed;
      vR = defaultSpeed;
      break;
    case B1100:
      vL = 0;
      vR = defaultSpeed;
      break;
    case B0100:
      vL = defaultSpeed/2;
      vR = defaultSpeed;
      break;
    case B0110:
      vL = defaultSpeed;
      vR = defaultSpeed;
      break;
    case B0010:
      vL = defaultSpeed;
      vR = defaultSpeed/2;
      break;
    case B0011:
      vL = defaultSpeed;
      vR = 0;
      break;
    case B0001:
      vL = defaultSpeed;
      vR = -defaultSpeed;
      break;
    case B1111: // alles zwart                  
      vL = defaultSpeed;
      vR = defaultSpeed;
      break;
    case B0000: // alles wit                 
      vL = 0;
      vR = 0;
      break;
  }
  servoL.writeMicroseconds(1500 + vL);
  servoR.writeMicroseconds(1500 - vR);
}

void on4wayJunction() {
  // eerst doorrijden tot we de horizontale zwarte lijn zien:
  while( scanLineSensors() != B1111 ) {
    startMovingForward(defaultSpeed);    
    delay(50);  
  }
  stopMoving();
  
  // nog een klein beetje doorrijden tot we in het midden van het kaartje staan:
  startMovingForward(defaultSpeed);    
  delay(50000/defaultSpeed); // = afstand/snelheid = tijd // TODO: afstand nog imperisch te bepalen
  stopMoving();
  
  // op een kruispunt kan hij dus naar 3 richtingen gaan
  
  // random bepalen welke richting hij moet nemen
  
  // maar... hij moet ook een beetje twijfelen. Dwz dat hij een paar richtingen moet aannemen voor hij uiteindelijk 'beslist' een nieuwe richting uit te gaan
  // hoeveel keer hij moet twijfelen wordt random bepaald:
  int directionsLength = random(2, 6); // == min=2, max=5
  
  // we maken een array met alle 'twijfelrichingen':
  int directions[directionsLength];
  
  // nu de richtingen bepalen (waarvan de laatste de echte gaat zijn):
  for(int d = 0; d < directionsLength; d++) {
    // random richting bepalen:
    // 0=naar links, 1=rechtdoor, 2=naar rechts:
    int directionNumber = random(0, 3); // == min=0, max=2
    
    switch(directionNumber) {
      case 0:
      directions[d] = -90;
      break;
      
      case 1:
      directions[d] = +0;
      break;
       
      case 2:
      directions[d] = +90;
      break;
    }
  }
  
  // nu echt die bewegingen maken:
  // opgelet, als hij bvb -90 graden doet, dan moet hij terug +180 doen om bvb naar +90 graden te gaan
  // dus bijhouden wat zijn huidige richting is:
  int currentDegrees = 0;
  
  for(int d = 0; d < directionsLength; d++) {
    int realDegreesToTurn = directions[d] - currentDegrees;
    turnDegrees(realDegreesToTurn);
    currentDegrees = directions[d];
  }
  
  // voila, de laatste waarde van de directions nemen we aan voor zijn uiteindelijke beslissing
  // deze functie eindigt en in de loop() functie zou hij dus moeten verder gaan met followLine()
}

void on3wayJunction() {
  
  
  
}

// driving functions:

void startMovingForward(int speed) {
  servoL.writeMicroseconds(1500 + speed);
  servoR.writeMicroseconds(1500 - speed);
}

void stopMoving() {
  servoL.writeMicroseconds(1500);
  servoR.writeMicroseconds(1500);
}

void turnAround() {
  int turnspeed = 50;  // natte vinger
  int turntime = 2200; // natte vinger
  
  servoL.writeMicroseconds(1500 + turnspeed);
  servoR.writeMicroseconds(1500 + turnspeed);
  delay(turntime);
  stopMoving();
}


void turnRight() {
  int turnspeed = 50;  // natte vinger
  int turntime = 1100; // natte vinger
  
  servoL.writeMicroseconds(1500 + turnspeed);
  servoR.writeMicroseconds(1500 + turnspeed);
  delay(turntime);
  stopMoving();
}

void turnLeft() {
  int turnspeed = 50;  // natte vinger
  int turntime = 1100; // natte vinger
  
  servoL.writeMicroseconds(1500 - turnspeed);
  servoR.writeMicroseconds(1500 - turnspeed);
  delay(turntime);
  stopMoving();
}

void turnDegrees(int degrees) {
  // een positief getal is tegen de klok in (duh:p)
  
  int turnspeed = 50;  // natte vinger
  
  int degreesAbsolute = degrees;
  if(degrees < 0) {
    degreesAbsolute = -degrees;
    turnspeed = -turnspeed;
  }
  
  int turntime = 1100/90*degreesAbsolute; // natte vinger
  
  servoL.writeMicroseconds(1500 - turnspeed);
  servoR.writeMicroseconds(1500 - turnspeed);
  delay(turntime);
  stopMoving();
}

// helper functions:

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



