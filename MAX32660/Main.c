* PAW3902 Optical Flow Sensor
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
 *
 *
 ******************************************************************************
 *
 * @file    main.c
 * @brief   PAW3902
 * @details This example uses the UART to print to a terminal and flashes an LED.
 */

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "mxc_config.h"
#include "mxc_delay.h"
#include "led.h"
#include "tmr.h"
#include "tmr_utils.h"
#include "math.h"
#include "uart.h"
#include "spi.h"
#include "pb.h"
#include "mxc_pins.h"
#include "nvic_table.h"
#include "board.h"
#include "gpio.h"
#include "armv7m_systick.h"
#include "lp.h"
#include "icc.h"
#include "PAW3902.h"

// Pin definitions
#define RST    21  // PAM3902 reset
#define MOT    PIN_2  // use as data ready interrupt
#define PAW3902_intPin PIN_2

#define CLOCK_DIVIDER     0    // Divide by 2^n

/***** Globals *****/
spi_req_t req;
volatile int spi_flag;
uint8_t tx_data[16], rx_data[16];

volatile uint8_t mode = lowlight;
int16_t deltaX, deltaY, Shutter;
volatile int motionDetect = 0, alarmFlag = 0;

uint8_t frameArray[1225], dataArray[12], SQUAL, RawDataSum = 0;
uint8_t count0 = 0, count1 = 0, count2 = 0, count3 = 0, iterations = 0;

/***** Functions *****/
/******************************************************************************/
void spi_cb(void *req, int error)
{
    spi_flag = error;
}

void SPI0_IRQHandler(void)
{
	SPI_Handler(SPI0A);
}

void delayMicroseconds(uint32_t time_us)
{
	TMR_Delay(MXC_TMR0, USEC(time_us), NULL); // TMR timer delay
}

void delay(uint32_t time_ms)
{
	TMR_Delay(MXC_TMR0, MSEC(time_ms), NULL); // TMR timer delay
}

void ledBlink(uint32_t duration)
{
	  LED_On(0); delay(duration); LED_Off(0);
}

void PAW3902_intHandler()
{
	motionDetect = 1;
}

void writeByte(uint8_t reg, uint8_t value)
{
  req.width = SPI17Y_WIDTH_1;
  req.ssel = 0;
  req.ssel_pol = SPI17Y_POL_LOW;
  req.bits = 8;
  req.len = 2;
  req.callback = spi_cb;

  uint16_t temp = (value << 8);
  tx_data[0] =  (temp | (reg | 0x80)); // register write must have a 1 in bit 7 position
  req.tx_data = &tx_data;
  req.deass = 1;     // don't keep nCS asserted after transaction
  uint8_t error = 0;
  if((error = SPI_MasterTrans(SPI0A, &req)) != 0) {printf("Error writing %d\n", error);}
}

void writeByteDelay(uint8_t reg, uint8_t value)
{
  writeByte(reg, value);
  delayMicroseconds(11);
}

uint8_t readByte(uint8_t reg)
{
  uint16_t temp = (0xFF << 8);
  tx_data[0] = (temp | (reg | 0x7000)); // register read must have a 0 in bit 7 position
  req.tx_data = &tx_data;
  req.rx_data = &rx_data;
  req.width = SPI17Y_WIDTH_1;
  req.ssel = 0;
  req.ssel_pol = SPI17Y_POL_LOW;
  req.bits = 8;
  req.len = 2;
  req.callback = NULL;

  req.deass = 1;     // keep nCS asserted after transaction
  uint8_t error = 0;
  if((error = SPI_MasterTrans(SPI0A, &req)) != 0) {printf("Error writing %d\n", error);}
  delayMicroseconds(2);

  return rx_data[0];
}

void readBurstMode(uint8_t * dataArray)
{
   /* Setup MOSI output pin. */
   gpio_cfg_t gpio_MOSI;
   gpio_MOSI.port = 0;
   gpio_MOSI.mask = MOSI;
   gpio_MOSI.pad = GPIO_PAD_NONE;
   gpio_MOSI.func = GPIO_FUNC_OUT;
   GPIO_Config(&gpio_MOSI);

   tx_data[0] =  0x16; // register write must have a 1 in bit 7 position
   req.tx_data = tx_data;
   req.rx_data = rx_data;
   req.width = SPI17Y_WIDTH_1;
   req.ssel = 0;
   req.bits = 8;
   req.len = 1;
   req.callback = NULL;

   req.deass = 0;     // keep nCS asserted after transaction
   SPI_MasterTrans(SPI0A, &req);
   GPIO_OutSet(&gpio_MOSI); // hold MOSI high during burst read
   delayMicroseconds(2);

   for(uint8_t ii = 0; ii < 12; ii++)
   {
    SPI_MasterTrans(SPI0A, &req);
    dataArray[ii] = rx_data[0];
   }

   GPIO_OutClr(&gpio_MOSI); // return MOSI to LOW
   delayMicroseconds(1);
}

//******************************************************************************

int main(void)
{
    int error;

    SPI_Shutdown(SPI0A);
    if((error = SPI_Init(SPI0A, 3, 2000000)) != E_NO_ERROR) {
        printf("Error initializing SPI Master %d.  (Error code = %d)\n", SPI0A, error);
        return 1;
    }


    // delay before uart shutdown
  //  while(UART_Busy(MXC_UART_GET_UART(CONSOLE_UART)));
   // Console_Shutdown();

	/* Set the system clock divider */
  //  MXC_SETFIELD(MXC_GCR->clkcn, MXC_F_GCR_CLKCN_PSC, (CLOCK_DIVIDER << MXC_F_GCR_CLKCN_PSC_POS));
    // set the VCORE Detect Bypass bit:
  //  MXC_SETBIT((MXC_BASE_PWRSEQ + MXC_R_PWRSEQ_LP_CTRL), MXC_F_PWRSEQ_LP_CTRL_VCORE_DET_BYPASS_POS);

    // Update SystemCoreClock variable after clock divider change
  //  SystemCoreClockUpdate();

    /* Adjust the operating voltage range
      *     Choices are:
        LP_OVR_0_9       = MXC_S_PWRSEQ_LP_CTRL_OVR_0_9V,    24 MHz
        LP_OVR_1_0       = MXC_S_PWRSEQ_LP_CTRL_OVR_1_0V,    48 MHz
        LP_OVR_1_1       = MXC_S_PWRSEQ_LP_CTRL_OVR_1_1V,    96 MHz default
    */
 //   LP_SetOperatingVoltage(LP_OVR_0_9); // set lowest voltage/clock speed
    /* Re-initialize the console uart to get correct baud rates */
 	Console_Init();

	ICC_Enable(); // enable instruction cache for higher efficiency

	PAW3902begin();

	  // Check device ID as a test of SPI communications
      if (!checkID()) {
      printf("Initialization of the opticalFlow sensor failed\n");
      printf(" \n");
      while(1) { }
      }

      setMode(bright);

      // Configure sensor interrupts
      gpio_cfg_t gpio_interrupt1;
      gpio_interrupt1.port = PORT_0;
      gpio_interrupt1.mask = PAW3902_intPin;
      gpio_interrupt1.pad = GPIO_PAD_PULL_DOWN;
      gpio_interrupt1.func = GPIO_FUNC_IN;
      GPIO_Config(&gpio_interrupt1);
      GPIO_RegisterCallback(&gpio_interrupt1, PAW3902_intHandler, 0);
      GPIO_IntConfig(&gpio_interrupt1, GPIO_INT_EDGE, GPIO_INT_RISING);
      GPIO_IntEnable(&gpio_interrupt1);
      NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(PORT_0));
      LP_EnableGPIOWakeup(&gpio_interrupt1);


    while(1)  // main do loop
    	 {

    	  // Navigation
    	  if(motionDetect)
    	  {
    	   motionDetect = 0;
    	   iterations++;

    	   readBurstMode(dataArray);
    	   deltaX = ((int16_t)dataArray[3] << 8) | dataArray[2];
    	   deltaY = ((int16_t)dataArray[5] << 8) | dataArray[4];
    	   SQUAL = dataArray[6];
    	   RawDataSum = dataArray[7];
    	   Shutter = ((uint16_t)dataArray[10] << 8) | dataArray[11];
    	   Shutter &= 0x1FFF;

    	   mode =    getMode();
    	   // Don't report data if under thresholds
    	   if((mode == bright       ) && (SQUAL < 25) && (Shutter >= 0x1FF0)) deltaX = deltaY = 0;
    	   if((mode == lowlight     ) && (SQUAL < 70) && (Shutter >= 0x1FF0)) deltaX = deltaY = 0;
    	   if((mode == superlowlight) && (SQUAL < 85) && (Shutter >= 0x0BC0)) deltaX = deltaY = 0;

    	   // Switch brightness modes automagically

    	   // switch from lowlight to bright when shutter value < 3000 for 10 iterations
    	   if((mode == lowlight) && (Shutter < 0x0BB8))
    	   {
    	       count0++;
    	       if(count0 >= 10) setMode(bright);
    	   }
    	   else
    	   {
    	       count0 = 0;
    	   }

    	   // switch from superlowlight to lowlight when shutter value < 1000 for ten iterations
    	   if((mode == superlowlight) && (Shutter < 0x03E8))
    	   {
    	       count1++;
    	       if(count1 >= 10) setMode(lowlight);
    	   }
    	   else
    	   {
    	       count1 = 0;
    	   }


    	   // switch from bright to lowlight when shutter value >= 7711 for ten iterations
    	   if((mode == bright) && (Shutter >= 0x1E1F) && (RawDataSum < 0x3C))
    	   {
    	       count2++;
    	       if(count2 >= 10) setMode(lowlight);
    	   }
    	   else
    	   {
    	       count2 = 0;
    	   }

    	   // switch from lowlight to superlowlight when shutter value >= 7711 for ten iterations
    	   if((mode == lowlight) && (Shutter >= 0x1E1F) && (RawDataSum < 0x5A))
    	   {
    	       count3++;
    	       if(count3 >= 10) setMode(superlowlight);
    	   }
    	   else
    	   {
    	       count3 = 0;
    	   }

    	   // Drop out of superlowlight mode as soon as the Shutter less than 500
    	   if((mode == superlowlight) && (Shutter < 0x01F4)) setMode(lowlight);

    	   printf("X: %.2f", deltaX); printf(", Y: %.2f\n", deltaY);
    	   printf("SQUAL: %.2f", SQUAL);printf(", Shutter: 0x%x\n", Shutter);
    	   printf("RawDataSum: 0x%x", RawDataSum);printf(", mode: %x\n", mode);
    	   printf("  \n");
    	  }

    	ledBlink(100);
        delay(900);
    	}

}
