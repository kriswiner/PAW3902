/* 11/11/2019 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer
 *  
 This sketch is to operate the 
 
 The sketch uses default SPI pins on the Dragonfly development board.

 Sketch modeled after Bitcraze PMW3901 library-https://github.com/bitcraze/PMW3901

 Library may be used freely and without limit with attribution.
 
  */
#include <SPI.h>
#include "PAW3902.h"

// Pin definitions
#define myLed  26  // blue led on Dragonfly
#define CSPIN  10  // default chip select for SPI
#define EN     31  // board enable is pulled up, set to TTL LOW to disable
#define RST    21  // PAM3902 reset
#define MOT    30  // use as data ready interrupt

uint8_t mode = lowlight; // mode choices are bright, lowlight (default), superlowlight
int16_t deltaX, deltaY;
bool motionDetect = false, alarmFlag = false;
uint8_t status;
uint8_t dataArray[12], SQUAL;
uint16_t Shutter;

PAW3902 opticalFlow(CSPIN); // Instantiate PAW3902

void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("Serial enabled!");

  // Configure led
  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH); // start with led off since active LOW on Dragonfly

  pinMode(MOT, INPUT); // data ready interrupt

  // Configure SPI Flash chip select
  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH);

  SPI.begin(); // initiate SPI 
  delay(1000);

  digitalWrite(myLed, LOW);

  opticalFlow.begin();  // Prepare SPI port and restart device
  opticalFlow.reset();  // Reset device
  
  // Check device ID as a test of SPI communications
  if (!opticalFlow.checkID()) {
  Serial.println("Initialization of the opticalFlow sensor failed");
  while(1) { }
  
  opticalFlow.setMode(mode); // set device mode

  digitalWrite(myLed, HIGH);
  }

  attachInterrupt(MOT, myIntHandler, FALLING); // active LOW 
  status = opticalFlow.status();  // clear interrupt before entering main loop
  /* end of setup */

}

void loop() {
  
  if(motionDetect)
  {
   motionDetect = false;
   
   status = opticalFlow.status();
   opticalFlow.readMotionCount(&deltaX, &deltaY, &SQUAL, &Shutter); 
//   opticalFlow.readBurstMode(dataArray);
//   deltaX = ((int16_t)dataArray[3] << 8) | dataArray[2];
//   deltaY = ((int16_t)dataArray[5] << 8) | dataArray[4];
//   SQUAL = dataArray[6];
//   Shutter = (uint16_t)((dataArray[11] & 0x1F) << 8) | dataArray[10];
   Serial.print("X: ");Serial.print(deltaX);Serial.print(", Y: ");Serial.println(deltaY);
   Serial.print("SQUAL: 0x");Serial.print(SQUAL, HEX);Serial.print(", Shutter: 0x");Serial.println(Shutter & 0x1FFF, HEX);
  }
  delay(100);
  }


void myIntHandler()
{
  motionDetect = true;
}
