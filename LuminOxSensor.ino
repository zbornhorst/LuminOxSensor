#include <Time.h>
#include <SoftwareSerial.h>

#define _SENSOR_SERIAL_RX_ 2
#define _SENSOR_SERIAL_TX_ 3

#define _SST_BUFFER_LENGTH_ 50
#define _MAX_STRLEN_ 130

const boolean isInverted = true;

SoftwareSerial sensorSerial (_SENSOR_SERIAL_RX_, _SENSOR_SERIAL_TX_, isInverted);
 
char buffer[_SST_BUFFER_LENGTH_];
char strBuf[_MAX_STRLEN_];

uint8_t ind; 

double oVal;
double tVal;
int pVal;
double pctVal;
int eVal;
int errorCode;

time_t lastTime;
time_t curTime;

void setup () {
  Serial.begin (9600);
  sensorSerial.begin (9600);
  // analogReference (EXTERNAL); //connect AREF to a 3.3V power supply? Will probably work on standard pins instead
  Serial.println ("ppO2 (mBar), temp (C), pressure (mBar), O2 in percent (%), status");
  sprintf(strBuf, "M 0\r\n");
  sensorSerial.write(strBuf); //puts sensor in stream mode
  
  lastTime = now();
}

void loop () {
  char lastChar = 0x01; //Initialize to anything other than 0x0A
  ind = 0; //Reset index counter
  
  //If any data available, then make sure to read the whole line
  if (sensorSerial.available()) {
    //Read until end of line or exceeding buffer
    //0x0A is \n
    while ((lastChar != 0x0A) && (ind < _SST_BUFFER_LENGTH_)) { 
      if (sensorSerial.available()) { //Check that there is another character
        lastChar = sensorSerial.read(); 
        buffer[ind++] = lastChar; 
      }
    }
   
    //Finish the string off and show it on the serial monitor 
    buffer[ind] = '\0'; //Add string ender to sprintf doesn't overrun
    sprintf(strBuf, "Received message: %s", buffer);
    Serial.println(strBuf);
    
    lastTime = now();
  }
  
  parseStreamMode(); 
  printValues();
  
  curTime = now();
  
  if ((curTime - lastTime) > 5) {
    sprintf(strBuf, "LuminOx WARNING:  No new messages in %ul seconds.", (curTime - lastTime));
  }
 }
 
void parseStreamMode () {
  char *bufPtr = &buffer[0];
  while (*bufPtr++ != 0x0A) {
          
    while (*bufPtr == 0x20) { //ignore white space 
      bufPtr++;
    }
     
    switch (*bufPtr++) {
      case 'O':
        oVal = atof(bufPtr);
        bufPtr += 7;
        break;
      case 'T':
        tVal = atof(bufPtr);
        bufPtr += 6;
        break;
      case 'P':
        pVal = atoi(bufPtr);
        bufPtr += 5;
        break;
      case '%':
        pctVal = atof(bufPtr);
        bufPtr += 7;
        break;
      case 'e':
        eVal = atoi(bufPtr);
        bufPtr += 5;
        break;
      case 'E':  //a command was not received properly
        errorCode = atoi(bufPtr);
        bufPtr += 3;
        break;
      default:
         Serial.println("Unknown command type"); 
    } //switch
      
  } //for loop
} //function
 

void printErrorCode() {
  
  switch (errorCode) {
    case 0:
      Serial.println ("RS232 Receiver Overflow");
    case 1:
      Serial.println ("Invalid Command");
      break;
    case 2:
      Serial.println ("Invalid Frame");
      break;
    case 3:
      Serial.println ("Invalid Argument");
      break;
    default:
      sprintf(strBuf, "Unknown error code received: %.2d", errorCode);
      Serial.println (strBuf);
  }
 }
 
 void printValues() {
   sprintf(strBuf, "ppO2 = %.1f mBar, T = %.1f C, P = %d mbar, pctO2 = %.2f\%, e = %.4d", 
           oVal, tVal, pVal, pctVal, eVal);
   Serial.println(strBuf);
   return;
 
}
