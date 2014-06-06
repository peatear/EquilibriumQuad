#include <SoftwareSerial.h>
#include <Servo.h>

Servo servo1;

//brushless esc wire 
//orange = signal
//red = plus
//brown is ground

const int TX_BT = 10;
const int RX_BT = 11;

const int signalPin = 3;

const int PIN_RED = 5;
const int PIN_GREEN = 6;
const int PIN_YELLOW = 7;

const int PIN_PROXIMITY_SENSOR = 3;

SoftwareSerial btSerial(TX_BT, RX_BT);

//Frequency to send periodic messages to Windows Phone, in milliseconds.
const unsigned long periodicMessageFrequency = 5000;
unsigned long time = 0;

//Process the incoming command from Windows Phone.
void processCommand(char* command) {
  if(strcmp(command,"TURN_ON_RED") == 0) {
    digitalWrite(PIN_RED, HIGH);
    sendMessage("LED_RED:ON");
  } 
}

//Send a message back to the Windows Phone.
void sendMessage(char* message) {
  int messageLen = strlen(message);
  if(messageLen < 256) {  
    btSerial.write(messageLen);
    btSerial.print(message);
  }
}

//Send a set of periodic messages to the Windows Phone.
//This message could be a sensor data, like a thermometer data.
void sendPeriodicMessages() {
  int valProximitySensor = digitalRead(PIN_PROXIMITY_SENSOR);
}

//Setup Arduino function
void setup() {
  Serial.begin(9600);
  Serial.println("USB Connected");
  btSerial.begin(9600);
  

  
  pinMode(signalPin, OUTPUT);
}

//Loop Arduino function
void loop() {
  if(btSerial.available()) {
      int commandSize = (int)btSerial.read();
      char command[commandSize];
      int commandPos = 0;
      while(commandPos < commandSize) {
        if(btSerial.available()) {
          command[commandPos] = (char)btSerial.read();
          commandPos++;
        }
      }
      command[commandPos] = 0;
      processCommand(command);
  }
  unsigned long currentTime = millis();
  if((currentTime - time) > periodicMessageFrequency) {
    sendPeriodicMessages();
    time = currentTime;
  }
}


