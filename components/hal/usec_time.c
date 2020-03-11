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
 * usec_time.c - microsecond-resolution timer and timestamps.
 */

#include "usec_time.h"
#include "esp_timer.h"

//static uint32_t usecTimerHighCount;

void initUsecTimer(void)
{

}

uint64_t usecTimestamp(void)
{

    return (uint64_t)esp_timer_get_time();

}


