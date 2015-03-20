#include <Servo.h>

#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// Pins:
// pin 0,1 hangen aan de seriele poort (is intern zo, niet los te koppelen)
// pin 4,5,6,7 hangen aan de line sensoren
// pin 10 hangt aan de ultrasone sensor
// pin 11 hangt aan de servo die de ultrasone sensor vasthoudt
// pin 12 hangt aan de rechter servo
// pin 13 hangt aan de linker servo

bool stopRobot = false;
bool ignoreNFC = false;


// Define RGB Led:
int ledR = 2;
int ledG = 3;
int ledB = 8;

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
  if (!ignoreNFC && nfc.tagPresent(10)) {
    Serial.println("Found tag");
//    NfcTag tag = nfc.read();
//    tag.print();

    // TODO: hier nog checken welke kaart het precies is, voorlopig veronderstellen we dat het altijd een kruispunt is
    // zie https://github.com/mixbe/NFC-2-BLE-on-RFDuino/blob/master/rfduino-readtag/rfduino-readtag.ino voor code om tagid's te vergelijken
    
    
    ignoreNFC = true;
    on4wayJunction();
    
  }else{
    if(!stopRobot) {
    
    
      followLine();
      
      
      
      // kijken of hij aan de rand van een kaartje zit:
      // B0000 = alles wit
      if(scanLineSensors() == B0000) {
        onEndOfCard();
      }
    
    }
    
    // terug bewegen als hij terug zwart ziet (opheffen bvb):
    if(scanLineSensors() == B1111) {
      stopRobot = false;
      ignoreNFC = false;
    }

    
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
  }
  servoL.writeMicroseconds(1500 + vL);
  servoR.writeMicroseconds(1500 - vR);
}

void on4wayJunction() {
  Serial.println("on4wayJunction");
  
  // eerst doorrijden tot we de horizontale zwarte lijn zien:
  while( scanLineSensors() != B1111 ) {
    startMovingForward(defaultSpeed);    
    delay(50);  
  }
  stopMoving();
  
  Serial.println("line gezien");

  // nog een klein beetje doorrijden tot we in het midden van het kaartje staan:
  startMovingForward(defaultSpeed);    
  delay(79000/defaultSpeed); // = afstand/snelheid = tijd // TODO: afstand nog imperisch te bepalen
  stopMoving();
  
  Serial.println("we zitten int midden");
  
  
  // op een kruispunt kan hij dus naar 3 richtingen gaan
  
  // random bepalen welke richting hij moet nemen
  
  // maar... hij moet ook een beetje twijfelen. Dwz dat hij een paar richtingen moet aannemen voor hij uiteindelijk 'beslist' een nieuwe richting uit te gaan
  // hoeveel keer hij moet twijfelen wordt random bepaald:
  int directionsLength = random(1, 5); // == min=1, max=3
   
//  int directionsLength = 1; // DEBUG: geen twijfelrichingen
  
  // we maken een array met alle 'twijfelrichingen':
  int directions[directionsLength];
  
  // nu de richtingen bepalen (waarvan de laatste de echte gaat zijn):
  for(int d = 0; d < directionsLength; d++) {
    // random richting bepalen:
    // 0=naar links, 1=rechtdoor, 2=naar rechts:
    int directionNumber = random(0, 3); // == min=0, max=2
    
    
//    directionNumber = 0; // DEBUG: vaste richting
    
    switch(directionNumber) {
      case 0:
      Serial.print("links ");
      directions[d] = +90;
      break;
      
      case 1:
      Serial.print("rechtoor ");
      directions[d] = +0;
      break;
       
      case 2:
      Serial.print("rechts ");
      directions[d] = -90;
      break;
    }
    
    Serial.println("");
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
  
  
  // DEBUG:
//  fullStop();
}

void on3wayJunction() {
  
  
  
}

void onEndOfCard() {
  // klein beetje achteruit rijden zodat hij in het midden van de kaart staat:
  
  startMovingBackward(defaultSpeed);    
  delay(28000/defaultSpeed); // = afstand/snelheid = tijd 
  stopMoving();
  
  stopRobot = true; //zodat hij ook in de loop() niets meer doet
}

// driving functions:

void startMovingForward(int speed) {
  servoL.writeMicroseconds(1500 + speed);
  servoR.writeMicroseconds(1500 - speed);
}

void startMovingBackward(int speed) {
  startMovingForward(-speed);
}

void stopMoving() {
  servoL.writeMicroseconds(1500);
  servoR.writeMicroseconds(1500);
}

void fullStop() {
  stopMoving();
  stopRobot = true;
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
  
  
  Serial.print("turn ");
  Serial.print(degrees);
  Serial.println(" degrees");
  
  int turnspeed = 50;  // natte vinger
  
  int degreesAbsolute = degrees;
  if(degrees < 0) {
    degreesAbsolute = -degrees;
    turnspeed = -turnspeed;
  }
  
  
  int turntime = 16*degreesAbsolute; // natte vinger
  
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



