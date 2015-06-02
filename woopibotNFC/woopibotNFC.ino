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


// Define RGB Led:
int ledR = 2;
int ledG = 3;
int ledB = 8;

// Servo stuff:
Servo servoL;                                
Servo servoR;


// Speeds:
int defaultLeftSpeed = 97; // nie veranderen, anders is alles omzeep
int defaultRightSpeed = 85;

// Laatste NFC kaart bijhouden (alsook uid size, want kan ook verschillend zijn):
byte lastNfcUid[20]; // genoeg bytes voorzien, geen idee of er zulke grote uid's bestaan
int lastNfcLength = 0;

int endOfCardCounter = 0;

bool firstCardIsJunction = true;

// EN MET BATTERIJEN IN SPELEN, anders is het ook omzeep

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
  if(stopRobot) {
    // terug bewegen als hij terug zwart ziet (opheffen bvb):
    if(scanLineSensors() == B1111) {
      Serial.println("reset all");
      stopRobot = false;
    }
    
    delay(50);
    return; //break uit loop()
  }

  // CHECK NFC:
  if (nfc.tagPresent(10)) {
    NfcTag tag = nfc.read();
    
    // TODO: hier nog checken welke kaart het precies is, voorlopig veronderstellen we dat het altijd een kruispunt is
    // zie https://github.com/mixbe/NFC-2-BLE-on-RFDuino/blob/master/rfduino-readtag/rfduino-readtag.ino voor code om tagid's te vergelijken
    
    int uidLength = tag.getUidLength();
    byte uid[uidLength];
    tag.getUid(uid, uidLength);

    
    if( uidLength != lastNfcLength || !isSameCard(uid, lastNfcUid, uidLength) ) {
      Serial.println("Found NEW NFC card");
      // tag.print();
      
      
      // store last card:
      lastNfcLength = uidLength;
      for(int i=0; i<uidLength; i++) {
        lastNfcUid[i] = uid[i];
      }
      
      
      
      bool findCenter = true;
      if(firstCardIsJunction)
        findCenter = false;
      
      on4wayJunction(findCenter);
      return; //break uit loop()
    }
  }
  
  firstCardIsJunction = false;

  
  // CHECK EINDE KAARTJE:
  if(scanLineSensors() == B0000 /* 0000 = wit */) {
    endOfCardCounter++;
    
    // moet minstens 3 keer na mekaar 0000 gezien hebben voor hij akkoord is met einde van kaart:
    if(endOfCardCounter == 3) {
      onEndOfCard();
      endOfCardCounter = 0;
      return; //break uit loop()
    }
    
  }else{
    endOfCardCounter = 0;
  }
  
  
  // RIJ DOOR:
  followLine();
  
  delay(40);
}

void followLine() {
  int pins = scanLineSensors();
  printLineSensors(pins);
  
  // default waarden = ga gewoon vooruit:
  int vL = defaultLeftSpeed;
  int vR = defaultRightSpeed;
  
  // based on line sensors:
  switch(pins) {
    case B1000:                        
      vL = -defaultLeftSpeed;
      vR = defaultRightSpeed;
      break;
    case B1100:
      vL = 0;
      vR = defaultRightSpeed;
      break;
    case B0100:
      vL = defaultLeftSpeed/2;
      vR = defaultRightSpeed;
      break;
    case B0110:
      vL = defaultLeftSpeed;
      vR = defaultRightSpeed;
      break;
    case B0010:
      vL = defaultLeftSpeed;
      vR = defaultRightSpeed/2;
      break;
    case B0011:
      vL = defaultLeftSpeed;
      vR = 0;
      break;
    case B0001:
      vL = defaultLeftSpeed;
      vR = -defaultRightSpeed;
      break;
    case B1111: // alles zwart                  
      vL = defaultLeftSpeed;
      vR = defaultRightSpeed;
      break;
  }
  servoL.writeMicroseconds(1500 + vL);
  servoR.writeMicroseconds(1500 - vR);
}

void on4wayJunction(bool findCenter) {
  Serial.println("on4wayJunction");
  
  if(findCenter) {
  
    // eerst doorrijden tot we de horizontale zwarte lijn zien:
    while( scanLineSensors() != B1111 ) {
      startMovingForward();    
      delay(50);  
    }
    stopMoving();
    
    Serial.println("line gezien");
  
    // nog een klein beetje doorrijden tot we in het midden van het kaartje staan:
    startMovingForward();    
    delay(412); // natte vinger
    stopMoving();
  
  }
  
  Serial.println("we zitten int midden");

  
  // op een kruispunt kan hij dus naar 3 richtingen gaan
  
  // random bepalen welke richting hij moet nemen
  
  // maar... hij moet ook een beetje twijfelen. Dwz dat hij een paar richtingen moet aannemen voor hij uiteindelijk 'beslist' een nieuwe richting uit te gaan
  // hoeveel keer hij moet twijfelen wordt random bepaald:
  int directionsLength = random(1, 3); // == min=1, max=2
   
  directionsLength = 2; // DEBUG: vast aantal twijfelrichingen
  
  // we maken een array met alle 'twijfelrichingen':
  int directions[directionsLength];
  
  // nu de richtingen bepalen (waarvan de laatste de echte gaat zijn):
  for(int d = 0; d < directionsLength; d++) {
    // random richting bepalen:
    // 0=naar links, 1=rechtdoor, 2=naar rechts:
    
    // geen 2 dezelfde richtingen na mekaar bedenken:
    int directionNumber = 1;
    while(true) {
      directionNumber = random(0, 3); // == min=0, max=2
      if(d == 0 || directionNumber != directions[d-1]) {
         break;
      }
    }
    
    //directionNumber = 0; // DEBUG: vaste richting
    directions[d] = directionNumber;
  }

  // echte graden van maken:
  for(int d = 0; d < directionsLength; d++) {
    switch(directions[d]) {
      case 0:
      Serial.print("links ");
      directions[d] = +90;
      break;
      
      case 1:
      Serial.print("rechtdoor ");
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
    
    switch(realDegreesToTurn) {
      case -180:
      Serial.println("2x turnRight()");
      turnRight();
      turnRight();
      break;
      
      case -90:
      Serial.println("1x turnRight()");
      turnRight();
      break;
       
      case 0:
      break;
      
      case 90:
      Serial.println("1x turnLeft()");
      turnLeft();
      break;
      
      case 180:
      Serial.println("2x turnLeft()");
      turnLeft();
      turnLeft();
      break;
    }
    
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
  
  startMovingBackward();    
  delay(480); // natte vinger
  stopMoving();
  
  stopRobot = true; //zodat hij ook in de loop() niets meer doet
}

// driving functions:

void startMovingForward() {
  servoL.writeMicroseconds(1500 + defaultLeftSpeed);
  servoR.writeMicroseconds(1500 - defaultRightSpeed);
}

void startMovingBackward() {
  servoL.writeMicroseconds(1500 - defaultLeftSpeed);
  servoR.writeMicroseconds(1500 + defaultRightSpeed);
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
  servoL.writeMicroseconds(1500 + defaultLeftSpeed);
  servoR.writeMicroseconds(1500 + defaultRightSpeed);
  delay(2200);
  stopMoving();
}


void turnRight() {
  servoL.writeMicroseconds(1500 + defaultLeftSpeed);
  servoR.writeMicroseconds(1500 + defaultRightSpeed);
  delay(710);
  stopMoving();
}

void turnLeft() {
  servoL.writeMicroseconds(1500 - defaultLeftSpeed);
  servoR.writeMicroseconds(1500 - defaultRightSpeed);
  delay(670);
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

boolean isSameCard(byte a[], byte b[], int array_size) {
   for (int i = 0; i < array_size; ++i)
     if (a[i] != b[i])
       return(false);
   return(true);
}



