* PAW3902.h
 *
 *  Created on: Nov 24, 2019
 *      Author: kris
 */
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

#if !defined(_PAW3902_H)
#define _PAW3902_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_config.h"
#include "mxc_delay.h"
#include "led.h"
#include "tmr.h"
#include "tmr_utils.h"
#include "spi.h"
#include "math.h"

#define bright        0
#define lowlight      1
#define superlowlight 2
#define CSPIN         ((uint32_t)(1UL << 7)) // PIN_7
#define MOSI          ((uint32_t)(1UL << 5)) // PIN_5

  void PAW3902begin();
  uint8_t PAW3902status();
  void initRegisters(uint8_t mode);
  void readMotionCount(int16_t *deltaX, int16_t *deltaY, uint8_t *SQUAL, uint16_t *Shutter);
  uint8_t checkID();
  void setMode(uint8_t mode);
  void reset();
  void shutdownPAW3902();
  uint8_t getMode();
  void enterFrameCaptureMode();
  void captureFrame(uint8_t * frameArray);
  void exitFrameCaptureMode();
  void initBright(void);
  void initLowLight(void);
  void initSuperLowLight(void);
  extern void delay(uint32_t time_ms);
  extern void delayMicroseconds(uint32_t time_us);
  volatile uint8_t _mode;

  extern void writeByte(uint8_t reg, uint8_t value);
  extern void writeByteDelay(uint8_t reg, uint8_t value);
  extern uint8_t readByte(uint8_t reg);
  extern void readBurstMode(uint8_t * dataArray);


#ifdef __cplusplus
}
#endif

#endif /* PAW3902_H_ */
