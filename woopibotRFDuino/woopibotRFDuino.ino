#include <RFduinoBLE.h>

int led = 3;


char bytes[1024];
int bytesIndex = 0;

void setup() {
  pinMode(led, OUTPUT);
  
  RFduinoBLE.deviceName = "woopibot";
  RFduinoBLE.advertisementData = "";
  
  Serial.begin(9600);
  
  RFduinoBLE.begin();
}


void loop() {
//  RFduinoBLE.send(0);

  readSerial();
  
  RFduino_ULPDelay(SECONDS(0.040));
}

void readSerial() {
  if(!Serial.available()) return;
  
  while(Serial.available()) {
    char val = Serial.read();
    bytes[bytesIndex++] = val;
  }
  
  RFduinoBLE.send(bytes, bytesIndex);
  bytesIndex = 0;
}

void RFduinoBLE_onDisconnect()
{
  digitalWrite(led, LOW);
}

void RFduinoBLE_onReceive(char *data, int len)
{
  digitalWrite(led, HIGH);
  Serial.print(data);
}
