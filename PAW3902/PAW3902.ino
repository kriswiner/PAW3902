/* 11/11/2019 Copyright Tlera Corporation
 *  
 *  Created by Kris Winer
 *  
 This sketch is to operate the PAw3902 optical flow sensor.
 
 The sketch uses default SPI pins on the Dragonfly development board.

 Sketch modeled after Bitcraze PMW3901 library-https://github.com/bitcraze/PMW3901 and
 PixArt PAW3902 Arduino sketch

 Library may be used freely and without limit with attribution.
 
  */
#include <SPI.h>
#include "PAW3902.h"

// Pin definitions
#define myLed  26  // blue led on Dragonfly
#define CSPIN  10  // default chip select for SPI
#define MOSI   11  // SPI MOSI pin on Dragonfly
#define EN     31  // board enable is pulled up, set to TTL LOW to disable
#define RST    21  // PAM3902 reset
#define MOT    30  // use as data ready interrupt

uint8_t mode = bright; // mode choices are bright, lowlight (default), superlowlight
int16_t deltaX, deltaY, Shutter;
bool motionDetect = false, alarmFlag = false;
uint8_t status;
uint8_t frameArray[1225], dataArray[12], SQUAL, RawDataSum = 0;
uint8_t count0 = 0, count1 = 0, count2 = 0, count3 = 0, iterations = 0;

PAW3902 opticalFlow(CSPIN); // Instantiate PAW3902

void setup() {
  Serial.begin(115200);
  delay(4000);
  Serial.println("Serial enabled!");

  // Configure led
  pinMode(myLed, OUTPUT);
  digitalWrite(myLed, HIGH); // start with led off since active LOW on Dragonfly

  pinMode(MOT, INPUT); // data ready interrupt

  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);

  // Configure SPI Flash chip select
  pinMode(CSPIN, OUTPUT);
  digitalWrite(CSPIN, HIGH);

  SPI.begin(); // initiate SPI 
  delay(1000);

  digitalWrite(myLed, LOW);

  opticalFlow.begin();  // Prepare SPI port and restart device
  
  // Check device ID as a test of SPI communications
  if (!opticalFlow.checkID()) {
  Serial.println("Initialization of the opticalFlow sensor failed");
  while(1) { }
  }

  opticalFlow.setMode(mode);

  digitalWrite(myLed, HIGH);

  attachInterrupt(MOT, myIntHandler, FALLING); // active LOW 
  status = opticalFlow.status();  // clear interrupt before entering main loop
  /* end of setup */
}

void loop() {
  
  // Navigation
  if(motionDetect)
  {
   motionDetect = false;
   iterations++;
   
//   status = opticalFlow.status();
//   opticalFlow.readMotionCount(&deltaX, &deltaY, &SQUAL, &Shutter); 

   opticalFlow.readBurstMode(dataArray);
   deltaX = ((int16_t)dataArray[3] << 8) | dataArray[2];
   deltaY = ((int16_t)dataArray[5] << 8) | dataArray[4];
   SQUAL = dataArray[6];
   RawDataSum = dataArray[7];
   Shutter = ((uint16_t)dataArray[10] << 8) | dataArray[11];
   Shutter &= 0x1FFF;

   mode =    opticalFlow.getMode();
   // Don't report data if under thresholds
   if((mode == bright       ) && (SQUAL < 25) && (Shutter >= 0x1FF0)) deltaX = deltaY = 0;
   if((mode == lowlight     ) && (SQUAL < 70) && (Shutter >= 0x1FF0)) deltaX = deltaY = 0;
   if((mode == superlowlight) && (SQUAL < 85) && (Shutter >= 0x0BC0)) deltaX = deltaY = 0;

   // Switch brightness modes automagically

   // switch from lowlight to bright when shutter value < 3000 for 10 iterations
   if((mode == lowlight) && (Shutter < 0x0BB8))
   {
       count0++;
       if(count0 >= 10) opticalFlow.setMode(bright);
   }
   else 
   {
       count0 = 0;
   }

   // switch from superlowlight to lowlight when shutter value < 1000 for ten iterations
   if((mode == superlowlight) && (Shutter < 0x03E8))
   {
       count1++;
       if(count1 >= 10) opticalFlow.setMode(lowlight);
   }
   else 
   {
       count1 = 0;
   }

   // switch from bright to lowlight when shutter value >= 7711 for ten iterations
   if((mode == bright) && (Shutter >= 0x1E1F) && (RawDataSum < 0x3C))
   {
       count2++;
       if(count2 >= 10) opticalFlow.setMode(lowlight);
   }
   else 
   {
       count2 = 0;
   }

   // switch from lowlight to superlowlight when shutter value >= 7711 for ten iterations
   if((mode == lowlight) && (Shutter >= 0x1E1F) && (RawDataSum < 0x5A))
   {
       count3++;
       if(count3 >= 10) opticalFlow.setMode(superlowlight);
   }
   else 
   {
       count3 = 0;
   }
   
   // Drop out of superlowlight mode as soon as the Shutter less than 500
   if((mode == superlowlight) && (Shutter < 0x01F4)) opticalFlow.setMode(lowlight);
   
   Serial.print("X: ");Serial.print(deltaX);Serial.print(", Y: ");Serial.println(deltaY);
   Serial.print("SQUAL: ");Serial.print(SQUAL);Serial.print(", Shutter: 0x");Serial.println(Shutter, HEX);
   Serial.print("RawDataSum: 0x");Serial.print(RawDataSum, HEX);Serial.print(", mode: ");Serial.println(mode); 
  }

  // Frame capture
  if(iterations >= 25) // capture one frame per 25 iterations of navigation
  {
    iterations = 0;
    Serial.println("Hold camera still for frame capture!");
    delay(4000);
    
    opticalFlow.enterFrameCaptureMode();
    
    for(uint8_t kk = 0; kk < 5; kk++) // capture 5 frames then go back to navigating
    {
      opticalFlow.captureFrame(frameArray);
      for(uint8_t ii = 0; ii < 35; ii++) // plot the frame data on the serial monitor (TFT display would be better)
      {
        Serial.print(ii); Serial.print(" "); 
        for(uint8_t jj = 0; jj < 35; jj++)
        {
        Serial.print(frameArray[ii*35 + jj]); Serial.print(" ");
        }
        Serial.println(" ");
      }
      Serial.println(" ");
    }
  
    opticalFlow.exitFrameCaptureMode(); // exit fram capture mode
    digitalWrite(RST, LOW); delay(10); digitalWrite(RST, HIGH); // toggle reset to return to navigation mode
    Serial.println("Back in Navigation mode!");
  }

  delay(100); // report at 10 Hz
  
} // end of main loop


void myIntHandler()
{
  motionDetect = true;
}
