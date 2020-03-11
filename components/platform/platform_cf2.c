/**
*
 * ESPlane Firmware
 * 
 * Copyright 2019-2020  Espressif Systems (Shanghai) 
 * Copyright (C) 2011-2018 Bitcraze AB
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
 * Platform functionality for the EP21 platform
 */

#define DEBUG_MODULE "PLATFORM"

#include <string.h>
#include "platform.h"
#include "debug_cf.h"


static platformConfig_t configs[] =  //多平台支持设计
{

  {
      .deviceType = "EP21",
      .deviceTypeName = "ESPLANE 2.1",
      .sensorImplementation = SensorImplementation_mpu6050_HMC5883L_MS5611,
      .physicalLayoutAntennasAreClose = false,
      .motorMap = motorMapDefaultBrushed,
 },
    
};

const platformConfig_t *platformGetListOfConfigurations(int *nrOfConfigs)
{
  *nrOfConfigs = sizeof(configs) / sizeof(platformConfig_t);
  return configs;
}

bool platformInitHardware()
{
  //TODO:
    return true;
}

// Config functions ------------------------

const char *platformConfigGetPlatformName()
{
  return "EP21";
}
