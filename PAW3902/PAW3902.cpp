/* PAW3902 Optical Flow Sensor
 * Copyright (c) 2019 Tlera Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
 
#include "PAW3902.h"
#include <SPI.h>

PAW3902::PAW3902(uint8_t cspin)
  : _cs(cspin)
{ }


boolean PAW3902::begin(void) 
{
  // Setup SPI port
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3)); // 2 MHz max SPI clock frequency

  // Make sure the SPI bus is reset
  digitalWrite(_cs, HIGH);
  delay(1);
  digitalWrite(_cs, LOW);
  delay(1);
  digitalWrite(_cs, HIGH);
  delay(1);

  reset();

  // Reading the motion registers one time
  for (uint8_t ii = 0; ii < 5; ii++)
  {
    readByte(0x02 + ii);
    delayMicroseconds(2);
  }

  _mode = lowlight;
  setMode(lowlight); // set mode to lowlight as default

  SPI.endTransaction();
}


void PAW3902::setMode(uint8_t mode) 
{
 if(mode == _mode) return;
 
 _mode = mode;
 reset();
 initRegisters(mode);
}


uint8_t PAW3902::getMode() 
{
 return _mode;
}


void PAW3902::initRegisters(uint8_t mode)
{
  switch(mode)
  {
  case 0: // Bright
  initBright();
  break;
  
  case 1: // Low Light
  initLowLight();
  break;

  case 2: // Super Low Light
  initSuperLowLight();
  break;
  }
}


boolean PAW3902::checkID()
{
  // check device ID
  uint8_t product_ID = readByte(0x00);
  uint8_t revision_ID = readByte(0x01);
  uint8_t inverse_product_ID = readByte(0x5F);

  Serial.print("Product ID = 0x"); Serial.print(product_ID, HEX); Serial.println(" should be 0x49");
  Serial.print("Revision ID = 0x0"); Serial.println(revision_ID, HEX); 
  Serial.print("Inverse Product ID = 0x"); Serial.print(inverse_product_ID, HEX); Serial.println(" should be 0xB6"); 

  if (product_ID != 0x49 && inverse_product_ID != 0xB6) return false;
  return true;
}


void PAW3902::reset()
{
  // Power on reset
  writeByte(0x3A, 0x5A);
  delay(1); 
}


void PAW3902::shutdown()
{
  // Shutdown
  writeByte(0x3B, 0xB6);
}



uint8_t PAW3902::status()
{
  uint8_t temp = readByte(0x02);
  return temp;
}


void PAW3902::readMotionCount(int16_t *deltaX, int16_t *deltaY, uint8_t *SQUAL, uint16_t *Shutter)
{
  *deltaX =  ((int16_t) readByte(0x04) << 8) | readByte(0x03);
  *deltaY =  ((int16_t) readByte(0x06) << 8) | readByte(0x05);
  *SQUAL =              readByte(0x07);
  *Shutter = ((uint16_t)readByte(0x0C) << 8) | readByte(0x0B);
}


void PAW3902::readBurstMode(uint8_t * dataArray)
{
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  
  digitalWrite(_cs, LOW);
  delayMicroseconds(1);
  
  SPI.transfer(0x16); // start burst mode
  digitalWrite(MOSI, HIGH); // hold MOSI high during burst read
  delayMicroseconds(2);
  
  for(uint8_t ii = 0; ii < 12; ii++)
  {
    dataArray[ii] = SPI.transfer(0);
  }
  digitalWrite(MOSI, LOW); // return MOSI to LOW
  digitalWrite(_cs, HIGH);
  delayMicroseconds(1);
  
  SPI.endTransaction();
  
}


void PAW3902::writeByte(uint8_t reg, uint8_t value) 
{
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  digitalWrite(_cs, LOW);
  delayMicroseconds(1);
  
  SPI.transfer(reg | 0x80);
  delayMicroseconds(10);
  SPI.transfer(value);
  delayMicroseconds(1);
  
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
}


void PAW3902::writeByteDelay(uint8_t reg, uint8_t value)
{
  writeByte(reg, value);
  delayMicroseconds(11);
}


uint8_t PAW3902::readByte(uint8_t reg) 
{
  SPI.beginTransaction(SPISettings(2000000, MSBFIRST, SPI_MODE3));
  digitalWrite(_cs, LOW);
  delayMicroseconds(1);
  
  SPI.transfer(reg & 0x7F);
  delayMicroseconds(2);
  
  uint8_t temp = SPI.transfer(0);
  delayMicroseconds(1);
  
  digitalWrite(_cs, HIGH);
  SPI.endTransaction();
  
  return temp;
}


// Performance optimization registers for the three different modes
void PAW3902::initBright()
{
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x55, 0x01);
  writeByteDelay(0x50, 0x07);
  writeByteDelay(0x7F, 0x0E);
  writeByteDelay(0x43, 0x10);
  writeByteDelay(0x48, 0x02);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x51, 0x7B);
  writeByteDelay(0x50, 0x00);
  writeByteDelay(0x55, 0x00);
  writeByteDelay(0x7F, 0x00);

  writeByteDelay(0x61, 0xAD);
  writeByteDelay(0x7F, 0x03);
  writeByteDelay(0x40, 0x00);
  writeByteDelay(0x7F, 0x05);
  writeByteDelay(0x41, 0xB3);
  writeByteDelay(0x43, 0xF1);
  writeByteDelay(0x45, 0x14);
  writeByteDelay(0x5F, 0x34);
  writeByteDelay(0x7B, 0x08);
  writeByteDelay(0x5E, 0x34);
  writeByteDelay(0x5B, 0x32);
  writeByteDelay(0x45, 0x17);
  writeByteDelay(0x70, 0xE5);
  writeByteDelay(0x71, 0xE5);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x44, 0x1B);
  writeByteDelay(0x40, 0xBF);
  writeByteDelay(0x4E, 0x3F);
  writeByteDelay(0x7F, 0x08);
  writeByteDelay(0x66, 0x44);
  writeByteDelay(0x65, 0x20);
  writeByteDelay(0x6A, 0x3A);
  writeByteDelay(0x61, 0x05);
  writeByteDelay(0x62, 0x05);
  writeByteDelay(0x7F, 0x09);
  writeByteDelay(0x4F, 0xAF);
  writeByteDelay(0x48, 0x80);
  writeByteDelay(0x49, 0x80);
  writeByteDelay(0x57, 0x77);
  writeByteDelay(0x5F, 0x40);
  writeByteDelay(0x60, 0x78);
  writeByteDelay(0x61, 0x78);
  writeByteDelay(0x62, 0x08);
  writeByteDelay(0x63, 0x50);
  writeByteDelay(0x7F, 0x0A);
  writeByteDelay(0x45, 0x60);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x4D, 0x11);
  writeByteDelay(0x55, 0x80);
  writeByteDelay(0x74, 0x21);
  writeByteDelay(0x75, 0x1F);
  writeByteDelay(0x4A, 0x78);
  writeByteDelay(0x4B, 0x78);
  writeByteDelay(0x44, 0x08);
  writeByteDelay(0x45, 0x50);
  writeByteDelay(0x64, 0xFE);
  writeByteDelay(0x65, 0x1F);
  writeByteDelay(0x72, 0x0A);
  writeByteDelay(0x73, 0x00);
  writeByteDelay(0x7F, 0x14);
  writeByteDelay(0x44, 0x84);
  writeByteDelay(0x65, 0x47);
  writeByteDelay(0x66, 0x18);
  writeByteDelay(0x63, 0x70);
  writeByteDelay(0x6F, 0x2C);
  writeByteDelay(0x7F, 0x15);
  writeByteDelay(0x48, 0x48);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x41, 0x0D);
  writeByteDelay(0x43, 0x14);
  writeByteDelay(0x4B, 0x0E);
  writeByteDelay(0x45, 0x0F);
  writeByteDelay(0x44, 0x42);
  writeByteDelay(0x4C, 0x80);
  writeByteDelay(0x7F, 0x10);
  writeByteDelay(0x5B, 0x03);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x40, 0x41);

  delay(10);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x32, 0x00);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x40, 0x40);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x68, 0x70);
  writeByteDelay(0x69, 0x01);
  writeByteDelay(0x7F, 0x0D);
  writeByteDelay(0x48, 0xC0);
  writeByteDelay(0x6F, 0xD5);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x5B, 0xA0);
  writeByteDelay(0x4E, 0xA8);
  writeByteDelay(0x5A, 0x50);
  writeByteDelay(0x40, 0x80);
  writeByteDelay(0x73, 0x1F);

  delay(10);
  writeByteDelay(0x73, 0x00);
}


 void PAW3902::initLowLight()   // default
{
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x55, 0x01);
  writeByteDelay(0x50, 0x07);
  writeByteDelay(0x7F, 0x0E);
  writeByteDelay(0x43, 0x10);
  writeByteDelay(0x48, 0x02);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x51, 0x7B);
  writeByteDelay(0x50, 0x00);
  writeByteDelay(0x55, 0x00);
  writeByteDelay(0x7F, 0x00);

  writeByteDelay(0x61, 0xAD);
  writeByteDelay(0x7F, 0x03);
  writeByteDelay(0x40, 0x00);
  writeByteDelay(0x7F, 0x05);
  writeByteDelay(0x41, 0xB3);
  writeByteDelay(0x43, 0xF1);
  writeByteDelay(0x45, 0x14);
  writeByteDelay(0x5F, 0x34);
  writeByteDelay(0x7B, 0x08);
  writeByteDelay(0x5E, 0x34);
  writeByteDelay(0x5B, 0x65);
  writeByteDelay(0x6D, 0x65);
  writeByteDelay(0x45, 0x17);
  writeByteDelay(0x70, 0xE5);
  writeByteDelay(0x71, 0xE5);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x44, 0x1B);
  writeByteDelay(0x40, 0xBF);
  writeByteDelay(0x4E, 0x3F);
  writeByteDelay(0x7F, 0x08);
  writeByteDelay(0x66, 0x44);
  writeByteDelay(0x65, 0x20);
  writeByteDelay(0x6A, 0x3A);
  writeByteDelay(0x61, 0x05);
  writeByteDelay(0x62, 0x05);
  writeByteDelay(0x7F, 0x09);
  writeByteDelay(0x4F, 0xAF);
  writeByteDelay(0x48, 0x80);
  writeByteDelay(0x49, 0x80);
  writeByteDelay(0x57, 0x77);
  writeByteDelay(0x5F, 0x40);
  writeByteDelay(0x60, 0x78);
  writeByteDelay(0x61, 0x78);
  writeByteDelay(0x62, 0x08);
  writeByteDelay(0x63, 0x50);
  writeByteDelay(0x7F, 0x0A);
  writeByteDelay(0x45, 0x60);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x4D, 0x11);
  writeByteDelay(0x55, 0x80);
  writeByteDelay(0x74, 0x21);
  writeByteDelay(0x75, 0x1F);
  writeByteDelay(0x4A, 0x78);
  writeByteDelay(0x4B, 0x78);
  writeByteDelay(0x44, 0x08);
  writeByteDelay(0x45, 0x50);
  writeByteDelay(0x64, 0xFE);
  writeByteDelay(0x65, 0x1F);
  writeByteDelay(0x72, 0x0A);
  writeByteDelay(0x73, 0x00);
  writeByteDelay(0x7F, 0x14);
  writeByteDelay(0x44, 0x84);
  writeByteDelay(0x65, 0x67);
  writeByteDelay(0x66, 0x18);
  writeByteDelay(0x63, 0x70);
  writeByteDelay(0x6F, 0x2C);
  writeByteDelay(0x7F, 0x15);
  writeByteDelay(0x48, 0x48);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x41, 0x0D);
  writeByteDelay(0x43, 0x14);
  writeByteDelay(0x4B, 0x0E);
  writeByteDelay(0x45, 0x0F);
  writeByteDelay(0x44, 0x42);
  writeByteDelay(0x4C, 0x80);
  writeByteDelay(0x7F, 0x10);
  writeByteDelay(0x5B, 0x03);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x40, 0x41);

  delay(10);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x32, 0x00);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x40, 0x40);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x68, 0x70);
  writeByteDelay(0x69, 0x01);
  writeByteDelay(0x7F, 0x0D);
  writeByteDelay(0x48, 0xC0);
  writeByteDelay(0x6F, 0xD5);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x5B, 0xA0);
  writeByteDelay(0x4E, 0xA8);
  writeByteDelay(0x5A, 0x50);
  writeByteDelay(0x40, 0x80);
  writeByteDelay(0x73, 0x1F);

  delay(10);
  writeByteDelay(0x73, 0x00);
}


 void PAW3902::initSuperLowLight()
{
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x55, 0x01);
  writeByteDelay(0x50, 0x07);
  writeByteDelay(0x7F, 0x0E);
  writeByteDelay(0x43, 0x10);
  writeByteDelay(0x48, 0x04);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x51, 0x7B);
  writeByteDelay(0x50, 0x00);
  writeByteDelay(0x55, 0x00);
  writeByteDelay(0x7F, 0x00);

  writeByteDelay(0x61, 0xAD);
  writeByteDelay(0x7F, 0x03);
  writeByteDelay(0x40, 0x00);
  writeByteDelay(0x7F, 0x05);
  writeByteDelay(0x41, 0xB3);
  writeByteDelay(0x43, 0xF1);
  writeByteDelay(0x45, 0x14);
  writeByteDelay(0x5F, 0x34);
  writeByteDelay(0x7B, 0x08);
  writeByteDelay(0x5E, 0x34);
  writeByteDelay(0x5B, 0x32);
  writeByteDelay(0x6D, 0x32);
  writeByteDelay(0x45, 0x17);
  writeByteDelay(0x70, 0xE5);
  writeByteDelay(0x71, 0xE5);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x44, 0x1B);
  writeByteDelay(0x40, 0xBF);
  writeByteDelay(0x4E, 0x3F);
  writeByteDelay(0x7F, 0x08);
  writeByteDelay(0x66, 0x44);
  writeByteDelay(0x65, 0x20);
  writeByteDelay(0x6A, 0x3A);
  writeByteDelay(0x61, 0x05);
  writeByteDelay(0x62, 0x05);
  writeByteDelay(0x7F, 0x09);
  writeByteDelay(0x4F, 0xAF);
  writeByteDelay(0x48, 0x80);
  writeByteDelay(0x49, 0x80);
  writeByteDelay(0x57, 0x77);
  writeByteDelay(0x5F, 0x40);
  writeByteDelay(0x60, 0x78);
  writeByteDelay(0x61, 0x78);
  writeByteDelay(0x62, 0x08);
  writeByteDelay(0x63, 0x50);
  writeByteDelay(0x7F, 0x0A);
  writeByteDelay(0x45, 0x60);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x4D, 0x11);
  writeByteDelay(0x55, 0x80);
  writeByteDelay(0x74, 0x21);
  writeByteDelay(0x75, 0x1F);
  writeByteDelay(0x4A, 0x78);
  writeByteDelay(0x4B, 0x78);
  writeByteDelay(0x44, 0x08);
  writeByteDelay(0x45, 0x50);
  writeByteDelay(0x64, 0xCE);
  writeByteDelay(0x65, 0x0B);
  writeByteDelay(0x72, 0x0A);
  writeByteDelay(0x73, 0x00);
  writeByteDelay(0x7F, 0x14);
  writeByteDelay(0x44, 0x84);
  writeByteDelay(0x65, 0x67);
  writeByteDelay(0x66, 0x18);
  writeByteDelay(0x63, 0x70);
  writeByteDelay(0x6F, 0x2C);
  writeByteDelay(0x7F, 0x15);
  writeByteDelay(0x48, 0x48);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x41, 0x0D);
  writeByteDelay(0x43, 0x14);
  writeByteDelay(0x4B, 0x0E);
  writeByteDelay(0x45, 0x0F);
  writeByteDelay(0x44, 0x42);
  writeByteDelay(0x4C, 0x80);
  writeByteDelay(0x7F, 0x10);
  writeByteDelay(0x5B, 0x02);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x40, 0x41);

  delay(25);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x32, 0x44);
  writeByteDelay(0x7F, 0x07);
  writeByteDelay(0x40, 0x40);
  writeByteDelay(0x7F, 0x06);
  writeByteDelay(0x68, 0x40);
  writeByteDelay(0x69, 0x02);
  writeByteDelay(0x7F, 0x0D);
  writeByteDelay(0x48, 0xC0);
  writeByteDelay(0x6F, 0xD5);
  writeByteDelay(0x7F, 0x00);
  writeByteDelay(0x5B, 0xA0);
  writeByteDelay(0x4E, 0xA8);
  writeByteDelay(0x5A, 0x50);
  writeByteDelay(0x40, 0x80);
  writeByteDelay(0x73, 0x0B);

  delay(25);
  writeByteDelay(0x73, 0x00);
}
