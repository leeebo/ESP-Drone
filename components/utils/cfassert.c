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
 * cfassert.c - Assert implementation
 */

#define DEBUG_MODULE "SYS"

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cfassert.h"
#include "led.h"
#include "motors.h"
#include "debug_cf.h"

#define MAGIC_ASSERT_INDICATOR 0x2f8a001f

typedef struct SNAPSHOT_DATA {
  uint32_t magicNumber;
  char* fileName;
  int line;
} SNAPSHOT_DATA;

// The .nzds section is not cleared at startup, data here will survive a
// reset (by the watch dog for instance)
//TODO:用于保存断点位置
 SNAPSHOT_DATA snapshot = { // __attribute__((section(".nzds"))) = {
  .magicNumber = 0,
  .fileName = "",
  .line = 0
};


void assertFail(char *exp, char *file, int line)
{
  portDISABLE_INTERRUPTS();
  storeAssertSnapshotData(file, line);
  DEBUG_PRINTD( "Assert failed %s:%d\n", file, line);

  motorsSetRatio(MOTOR_M1, 0);
  motorsSetRatio(MOTOR_M2, 0);
  motorsSetRatio(MOTOR_M3, 0);
  motorsSetRatio(MOTOR_M4, 0);

//TODO:
  ledClearAll();
  ledSet(ERR_LED1, 1);
  //ledSet(ERR_LED2, 1);

  while (1){
    printAssertSnapshotData(); 
    vTaskDelay(1000);   
  };
}

void storeAssertSnapshotData(char *file, int line)
{
  snapshot.magicNumber = MAGIC_ASSERT_INDICATOR;
  snapshot.fileName = file;
  snapshot.line = line;
}

void printAssertSnapshotData()
{
  if (MAGIC_ASSERT_INDICATOR == snapshot.magicNumber) {
    DEBUG_PRINT_LOCAL( "Assert failed at %s:%d\n", snapshot.fileName, snapshot.line);
    DEBUG_PRINT_LOCAL("Assert failed at %s:%d\n", snapshot.fileName, snapshot.line);
  } else {
    DEBUG_PRINT_LOCAL( "No assert information found\n");
    DEBUG_PRINT_LOCAL("Assert failed at %s:%d\n", snapshot.fileName, snapshot.line);
  }
}



