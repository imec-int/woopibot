#include <Servo.h>


// Pins:
// pin 0,1 hangen aan de seriele poort (is intern zo, niet los te koppelen)
// pin 4,5,6,7 hangen aan de line sensoren
// pin 10 hangt aan de ultrasone sensor
// pin 11 hangt aan de servo die de ultrasone sensor vasthoudt
// pin 12 hangt aan de rechter servo
// pin 13 hangt aan de linker servo, momenteel is dat pin 9 omdat pin 13 storing geeft bij het opladen van de code


const int statusLedPin = 13;

// Servo stuff:
Servo servoL;
Servo servoR;


// Speeds:
int vL = 0;
int vR = 0;

int fullspeed = 20;
int halfspeed = 10;

bool enableFollowline = true;

// onthou het laatste wat hij gezien heeft en stuur enkel door als je een verschil ziet:
int lastPins = B1111;

// front sensor and servo
const int pingSensor = 10;
Servo pingServo;

bool enablePingSensor = false;


// commands via Serial:
byte commandbytes[20];
int commandbytesIndex = 0;
bool commandStarted = false;


void setup() {
  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));

  Serial.begin(9600);                        // Set up Arduino Serial Monitor at 9600 baud


  // attach servos aan arduino pinnen:
  servoL.attach(9);
  servoR.attach(12);
  
//  pingServo.attach(11);


  // zet motors stil:
  servoL.writeMicroseconds(1500);
  servoR.writeMicroseconds(1500);

//  Serial.println("WOOPIBOT");

  delay(1000);

  pinMode(statusLedPin, OUTPUT);

  //servoL.write(90);
}

void loop() {
  // terug lopen als hij zwart ziet:
//  if(stopRobot) {
//    // terug bewegen als hij terug zwart ziet (opheffen bvb):
//    if(scanLineSensors() == B1111) { //zwart
//      Serial.println("reset all");
//      stopRobot = false;
//    }
//
//    delay(50);
//    return; //break uit loop()
//  }
//
//  // stoppen als hij wit ziet:
//  if(scanLineSensors() == B0000) { // wit
//    stopMoving();
//    stopRobot = true;
//    return; //break uit loop()
//  }





  // Read serial:
  readSerial();

  // RIJ DOOR:
  
  if(enableFollowline) {
    followLine();
  }


  sendLineSensors();
  
  if(enablePingSensor) {
     sendPingSensor();
  }

  delay(40);
}

void readSerial() {
  if(!Serial.available()) return;
  
  while(Serial.available()) {
    byte val = Serial.read();
    
    switch(val) {
      case 0x02:
      commandbytesIndex = 0;
      commandStarted = true;
      break;
      
      case 0x03:
      if(commandStarted) parseCommand(commandbytes, commandbytesIndex);
      commandStarted = false;
      break;
      
      default:
      if(commandStarted) commandbytes[commandbytesIndex++] = val;
      break;
    }
  }
}

void parseCommand(byte bytes[], int len) {
//  Serial.println("parsecommmand");
//  for(int i=0; i<len;i++) {
//    Serial.println((char)bytes[i]);
//  }
  
  if(len < 1) return;
  
  char command = bytes[0];
  if(command == 'L') {
     if(len < 5) return;
     
     int leftServo = getInt(bytes[1])*1000 + getInt(bytes[2])*100 + getInt(bytes[3])*10 + getInt(bytes[4])*1;
     servoL.writeMicroseconds(leftServo);
  }
  
  
  if(command == 'R') {
     if(len < 5) return;
     
     int rightServo = getInt(bytes[1])*1000 + getInt(bytes[2])*100 + getInt(bytes[3])*10 + getInt(bytes[4])*1;
     servoR.writeMicroseconds(rightServo);
  }
  
  if(command == 'F') {
     if(len < 2) return;
     
     if(bytes[1] == '0') {
       enableFollowline = false;
     }else{
       enableFollowline = true;
     }
  }
  
  
  if(command == 'P') {
     if(len < 2) return;
     
     if(bytes[1] == '0') {
       enablePingSensor = false;
     }else{
       enablePingSensor = true;
     }
  }
}

int getInt(char value) {
  int integer = value - '0'; // subtracts the Ascii value for '0' (48) to give the numeric value
  if (0 <= integer && integer <= 9) { // only values 0-9 are accepted
    return integer;
  }
  
  return 0;
}

void followLine() {
  int pins = scanLineSensors();
  bool stopDriving = false;
  
  // enkel 2 middelste sensoren gebruiken om de lijn te volgen:
  // de 2 buitenste kunnen we dan gebruiken voor iets anders
  switch(pins) {   
    case B1101:
    case B0101:
    case B1100:
    case B0100:
      vL = halfspeed;
      vR = fullspeed;
      break;
    
    case B0110:
      vL = fullspeed;
      vR = fullspeed;
      break;  
      
    case B0011:
    case B1011:
    case B1010:
    case B0010:
      vL = fullspeed;
      vR = halfspeed;
      break;
  
  
    case B0000: // alles wit
      stopDriving = true;
      break;
  }


  // -100 to 100 indicate course correction values
  // -100: full reverse; 0=stopped; 100=full forward
//  switch(pins) {
//    case B1000:
//      vL = -fullspeed;
//      vR = fullspeed;
//      break;
//    case B1100:
//      vL = 0;
//      vR = fullspeed;
//      break;
//    case B0100:
//      vL = halfspeed;
//      vR = fullspeed;
//      break;
//    case B0110:
//      vL = fullspeed;
//      vR = fullspeed;
//      break;
//    case B0010:
//      vL = fullspeed;
//      vR = halfspeed;
//      break;
//    case B0011:
//      vL = fullspeed;
//      vR = 0;
//      break;
//    case B0001:
//      vL = fullspeed;
//      vR = -fullspeed;
//      break;
//    case B0000: // alles wit
//      stopDriving = true;
//      break;
//  }



  if(!stopDriving) {
    servoL.writeMicroseconds(1500 + vL);
    servoR.writeMicroseconds(1500 - vR);
  }else{
    servoL.writeMicroseconds(1500);
    servoR.writeMicroseconds(1500);
  }

}

void sendLineSensors() {
  int pins = scanLineSensors();
  
  if(pins != lastPins) {
    Serial.write(0x02);
    Serial.print("S");
    printLineSensors(pins);
    Serial.write(0x03);
    lastPins = pins;
  }
}

void sendPingSensor() {
  int microseconds = ping(pingSensor);
  Serial.write(0x02);
  Serial.print("P");
  Serial.print(microseconds);
  Serial.write(0x03);
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

/*
 * Initiate and capture Ping))) Ultrasonic Distance Sensor's round trip echo time.
 * Parameter: pin - Digital I/O pin connected to Ping)))
 * Returns:   duration - The echo time in microseconds 
 * Source:    Ping by David A. Mellis, located in File -> Examples -> Sensors
 * To-Do:     Double check timing against datasheet
 */ 
long ping(int pin) {
  long duration;                                   // Variables for calculating distance
  pinMode(pin, OUTPUT);                            // I/O pin -> output
  digitalWrite(pin, LOW);                          // Start low
  delayMicroseconds(2);                            // Stay low for 2 us
  digitalWrite(pin, HIGH);                         // Send 5 us high pulse
  delayMicroseconds(5);                            
  digitalWrite(pin, LOW);                          
  pinMode(pin, INPUT);                             // Set I/O pin to input 
  duration = pulseIn(pin, HIGH, 25000);            // Measure echo time pulse from Ping)))
  return duration;                                 // Return pulse duration
}

void printLineSensors(int pins) {
  // pad with zeros:
  if (pins < 2) Serial.print(B0);
  if (pins < 4) Serial.print(B0);
  if (pins < 8) Serial.print(B0);
  Serial.print(pins, BIN);
}

boolean isSameCard(byte a[], byte b[], int array_size) {
   for (int i = 0; i < array_size; ++i)
     if (a[i] != b[i])
       return(false);
   return(true);
}



