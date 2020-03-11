/**
*
 * ESPlane Firmware
 * 
 * Copyright 2019-2020  Espressif Systems (Shanghai) 
 * Copyright (C) 2011-2012 Bitcraze AB
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, in version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * uart1.c - uart1 driver
 */
#include <string.h>

/*FreeRtos includes*/
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "esp32_bridge.h"

#include "config.h"
//#include "nvic.h"
#include "uart1.h"
#include "cfassert.h"
#include "config.h"
//#include "nvicconf.h"

/** This uart is conflicting with SPI2 DMA used in sensors_bmi088_spi_bmp388.c
 *  which is used in CF-RZR. So for other products this can be enabled.
 */
//#define ENABLE_UART1_DMA

static xQueueHandle uart1queue;

static bool isInit = false;
static bool hasOverrun = false;

#ifdef ENABLE_UART1_DMA
static xSemaphoreHandle uartBusy;
static xSemaphoreHandle waitUntilSendDone;
static DMA_InitTypeDef DMA_InitStructureShare;
static uint8_t dmaBuffer[64];
static bool    isUartDmaInitialized;
static uint32_t initialDMACount;
#endif

/**
  * Configures the UART DMA. Mainly used for FreeRTOS trace
  * data transfer.
  */
static void uart1DmaInit(void)
{
#ifdef ENABLE_UART1_DMA
  // TODO:

  isUartDmaInitialized = true;
#endif
}

void uart1Init(const uint32_t baudrate)
{


    uart1queue = xQueueCreate(64, sizeof(uint8_t));

  isInit = true;
}

bool uart1Test(void)
{
  return isInit;
}

bool uart1GetDataWithTimout(uint8_t *c)
{
  if (xQueueReceive(uart1queue, c, UART1_DATA_TIMEOUT_TICKS) == pdTRUE)
  {
    return true;
  }

  *c = 0;
  return false;
}

void uart1SendData(uint32_t size, uint8_t* data)
{
 // uint32_t i;

  if (!isInit)
    return;
//TODO:
  // for(i = 0; i < size; i++)
  // {
  //   while (!(UART1_TYPE->SR & USART_FLAG_TXE));
  //   UART1_TYPE->DR = (data[i] & 0x00FF);
  // }
}

#ifdef ENABLE_UART1_DMA
//TODO:
#endif

int uart1Putchar(int ch)
{
    uart1SendData(1, (uint8_t *)&ch);
    
    return (unsigned char)ch;
}

void uart1Getchar(char * ch)
{
  xQueueReceive(uart1queue, ch, portMAX_DELAY);
}

bool uart1DidOverrun()
{
  bool result = hasOverrun;
  hasOverrun = false;

  return result;
}

#ifdef ENABLE_UART1_DMA
//TODO:
#endif

void __attribute__((used)) USART3_IRQHandler(void)
{

}
