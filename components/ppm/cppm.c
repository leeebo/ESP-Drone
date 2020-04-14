/**
 *
 * ESPlane Firmware
 *
 * Copyright 2019-2020  Espressif Systems (Shanghai)
 * Copyright (C) 2011-2013 Bitcraze AB
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
 *
 * cppm.c - Combined PPM / PPM-Sum driver
 */

/* FreeRtos includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "stm32_legacy.h"
#include "cppm.h"
#include "nvicconf.h"
#include "commander.h"

#define DEBUG_MODULE  "CPPM"
#include "debug_cf.h"
#include "log.h"

#define CPPM_TIM_PRESCALER           (84 - 1) // TIM14 clock running at sysclk/2. Will give 1us tick.

#define CPPM_MIN_PPM_USEC            1150
#define CPPM_MAX_PPM_USEC            1900

static xQueueHandle captureQueue;
static uint16_t prevCapureVal;
static bool captureFlag;
static bool isAvailible;

void cppmInit(void)
{
    // captureQueue = xQueueCreate(64, sizeof(uint16_t));
// for ppm receiver

}

bool cppmIsAvailible(void)
{
    return isAvailible;
}

int cppmGetTimestamp(uint16_t *timestamp)
{
    ASSERT(timestamp);

    return xQueueReceive(captureQueue, timestamp, portMAX_DELAY);
}

void cppmClearQueue(void)
{
    xQueueReset(captureQueue);
}

float cppmConvert2Float(uint16_t timestamp, float min, float max)
{
    if (timestamp < CPPM_MIN_PPM_USEC) {
        timestamp = CPPM_MIN_PPM_USEC;
    }

    if (timestamp > CPPM_MAX_PPM_USEC) {
        timestamp = CPPM_MAX_PPM_USEC;
    }

    float scale = (float)(timestamp - CPPM_MIN_PPM_USEC) / (float)(CPPM_MAX_PPM_USEC - CPPM_MIN_PPM_USEC);

    return min + ((max - min) * scale);
}

uint16_t cppmConvert2uint16(uint16_t timestamp)
{
    if (timestamp < CPPM_MIN_PPM_USEC) {
        timestamp = CPPM_MIN_PPM_USEC;
    }

    if (timestamp > CPPM_MAX_PPM_USEC) {
        timestamp = CPPM_MAX_PPM_USEC;
    }

    uint16_t base = (timestamp - CPPM_MIN_PPM_USEC);

    return base * (65535 / (CPPM_MAX_PPM_USEC - CPPM_MIN_PPM_USEC));
}

void __attribute__((used)) TIM8_TRG_COM_TIM14_IRQHandler()
{
    //TODO:
}
