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

#ifndef __PAW3902_H
#define __PAW3902_H

#include "Arduino.h"

#include <stdint.h>

#define bright        0
#define lowlight      1
#define superlowlight 2

class PAW3902 {
public:
  PAW3902(uint8_t cspin);
  boolean begin(void);
  uint8_t status();
  void initRegisters(uint8_t mode);
  void readMotionCount(int16_t *deltaX, int16_t *deltaY, uint8_t *SQUAL, uint16_t *Shutter);
  void readBurstMode(uint8_t * dataArray); 
  boolean checkID();
  void setMode(uint8_t mode);
  void reset();
  void shutdown();
  uint8_t getMode();

private:
  uint8_t _cs, _mode;
  void writeByte(uint8_t reg, uint8_t value);
  void writeByteDelay(uint8_t reg, uint8_t value);
  uint8_t readByte(uint8_t reg);
  void initBright(void);
  void initLowLight(void);
  void initSuperLowLight(void);
};

#endif //__PAW3902_H

