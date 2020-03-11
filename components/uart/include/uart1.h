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
 * uart1.h - uart1 driver for deck port
 */
#ifndef UART1_H_
#define UART1_H_

#include <stdbool.h>
#include "eprintf.h"
#include "freertos/FreeRTOS.h"

#define UART1_BAUDRATE           9600
#define UART1_DATA_TIMEOUT_MS    1000
#define UART1_DATA_TIMEOUT_TICKS (UART1_DATA_TIMEOUT_MS / portTICK_RATE_MS)


/**
 * Initialize the UART.
 */
void uart1Init(const uint32_t baudrate);

/**
 * Test the UART status.
 *
 * @return true if the UART is initialized
 */
bool uart1Test(void);

/**
 * Read a byte of data from incoming queue with a timeout defined by UART1_DATA_TIMEOUT_MS
 * @param[out] c  Read byte
 * @return true if data, false if timeout was reached.
 */
bool uart1GetDataWithTimout(uint8_t *c);

/**
 * Sends raw data using a lock. Should be used from
 * exception functions and for debugging when a lot of data
 * should be transfered.
 * @param[in] size  Number of bytes to send
 * @param[in] data  Pointer to data
 */
void uart1SendData(uint32_t size, uint8_t* data);

/**
 * Sends raw data using DMA transfer.
 * @param[in] size  Number of bytes to send
 * @param[in] data  Pointer to data
 */
void uart1SendDataDmaBlocking(uint32_t size, uint8_t* data);

/**
 * Send a single character to the serial port using the uartSendData function.
 * @param[in] ch Character to print. Only the 8 LSB are used.
 *
 * @return Character printed
 */
int uart1Putchar(int ch);

void uart1Getchar(char * ch);

/**
 * Returns true if an overrun condition has happened since initialization or
 * since the last call to this function.
 * 
 * @return true if an overrun condition has happened
 */
bool uart1DidOverrun();

/**
 * Uart printf macro that uses eprintf
 * @param[in] FMT String format
 * @param[in] ... Parameters to print
 *
 * @note If UART Crtp link is activated this function does nothing
 */
#define uart1Printf(FMT, ...) eprintf(uart1Putchar, FMT, ## __VA_ARGS__)

#endif /* UART1_H_ */
