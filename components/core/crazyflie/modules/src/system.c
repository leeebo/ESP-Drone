/*
 *    ||          ____  _ __
 * +------+      / __ )(_) /_______________ _____  ___
 * | 0xBC |     / __  / / __/ ___/ ___/ __ `/_  / / _ \
 * +------+    / /_/ / / /_/ /__/ /  / /_/ / / /_/  __/
 *  ||  ||    /_____/_/\__/\___/_/   \__,_/ /___/\___/
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
 * system.c - Top level module implementation
 */

#include <stdbool.h>

/* FreeRTOS includes */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "version.h"
#include "config.h"
#include "param.h"
#include "log.h"
#include "ledseq.h"
#include "adc_esp32.h"
#include "pm_esplane.h"
#include "config.h"
#include "system.h"
#include "platform.h"
#include "configblock.h"
#include "worker.h"
#include "freeRTOSdebug.h"
//#include "uart1.h"
//#include "uart2.h"
#include "wifi_esp32.h"
#include "comm.h"
#include "stabilizer.h"
#include "commander.h"
#include "console.h"
#include "wifilink.h"
#include "mem.h"
//#include "proximity.h"
//#include "watchdog.h"
#include "queuemonitor.h"
#include "buzzer.h"
#include "sound.h"
#include "sysload.h"
#include "estimator_kalman.h"
//#include "deck.h"
//#include "extrx.h"
#include "app.h"
#include "stm32_legacy.h"
#define DEBUG_MODULE "SYS"
#include "debug_cf.h"
#include "static_mem.h"

/* Private variable */
static bool selftestPassed;
static bool canFly;
static bool isInit;

STATIC_MEM_TASK_ALLOC(systemTask, SYSTEM_TASK_STACKSIZE);

/* System wide synchronisation */
xSemaphoreHandle canStartMutex;
static StaticSemaphore_t canStartMutexBuffer;

/* Private functions */
static void systemTask(void *arg);

/* Public functions */
void systemLaunch(void)
{
  STATIC_MEM_TASK_CREATE(systemTask, systemTask, SYSTEM_TASK_NAME, NULL, SYSTEM_TASK_PRI);
}

// This must be the first module to be initialized!
void systemInit(void)
{
  if(isInit)
    return;

  DEBUG_PRINT_LOCAL("----------------------------\n");
  DEBUG_PRINT_LOCAL("%s is up and running!\n", platformConfigGetDeviceTypeName());

  canStartMutex = xSemaphoreCreateMutexStatic(&canStartMutexBuffer);
  xSemaphoreTake(canStartMutex, portMAX_DELAY);

  wifilinkInit();
  sysLoadInit();

  /* Initialized here so that DEBUG_PRINT (buffered) can be used early */
  debugInit();
  crtpInit();
  consoleInit();


  configblockInit();
  workerInit();
  adcInit();
  ledseqInit();
  pmInit();
  buzzerInit();
  DEBUG_PRINT_LOCAL("----------------------------\n");

#ifdef APP_ENABLED
  appInit();
#endif

  isInit = true;
}

bool systemTest()
{
  bool pass=isInit;

  pass &= ledseqTest();
  pass &= pmTest();
  DEBUG_PRINTD("pmTest = %d", pass);
  pass &= workerTest();
  DEBUG_PRINTD("workerTest = %d", pass);
  //pass &= buzzerTest();
  return pass;
}

/* Private functions implementation */

void systemTask(void *arg)
{
  bool pass = true;

  ledInit();
  wifiInit();
  vTaskDelay(M2T(500));

#ifdef DEBUG_QUEUE_MONITOR
  queueMonitorInit();
#endif

#ifdef ENABLE_UART1
  uart1Init(9600);
#endif
#ifdef ENABLE_UART2
  uart2Init(115200);
#endif

  //Init the high-levels modules
  systemInit();
  commInit();
  commanderInit();

  StateEstimatorType estimator = kalmanEstimator;
  estimatorKalmanTaskInit();
  //deckInit();
  //estimator = deckGetRequiredEstimator();
  stabilizerInit(estimator);
  //if (deckGetRequiredLowInterferenceRadioMode() && platformConfigPhysicalLayoutAntennasAreClose())
  //{
  //  platformSetLowInterferenceRadioMode();
  //}
  soundInit();
  memInit();

#ifdef PROXIMITY_ENABLED
  proximityInit();
#endif

	/* Test each modules */
  pass &= wifiTest();
  DEBUG_PRINTI("wifilinkTest = %d ", pass);
  pass &= systemTest();
  DEBUG_PRINTI("systemTest = %d ", pass);
  pass &= configblockTest();
  DEBUG_PRINTI("configblockTest = %d ", pass);
  pass &= commTest();
  DEBUG_PRINTI("commTest = %d ", pass);
  pass &= commanderTest();
  DEBUG_PRINTI("commanderTest = %d ", pass);
  pass &= stabilizerTest();
  DEBUG_PRINTI("stabilizerTest = %d ", pass);
  pass &= estimatorKalmanTaskTest();
  DEBUG_PRINTI("estimatorKalmanTaskTest = %d ", pass);
  //pass &= soundTest();
  pass &= memTest();
  DEBUG_PRINTI("estimatorKalmanTaskTest = %d ", pass);
  //pass &= watchdogNormalStartTest();

  //Start the firmware
  if(pass)
  {
    selftestPassed = 1;
    systemStart();
    DEBUG_PRINTI("systemStart ! selftestPassed = %d", selftestPassed);
    soundSetEffect(SND_STARTUP);
    ledseqRun(SYS_LED, seq_alive);
    ledseqRun(LINK_LED, seq_testPassed);
  }
  else
  {
    selftestPassed = 0;
    if (systemTest())
    {
      while(1)
      {
        ledseqRun(SYS_LED, seq_testPassed); //Red passed == not passed!
        vTaskDelay(M2T(2000));
        // System can be forced to start by setting the param to 1 from the cfclient
        if (selftestPassed)
        {
	        DEBUG_PRINT("Start forced.\n");
          systemStart();
          break;
        }
      }
    }
    else
    {
      ledInit();
      ledSet(SYS_LED, true);
    }
  }
  DEBUG_PRINT("Free heap: %d bytes\n", xPortGetFreeHeapSize());

  workerLoop();

  //Should never reach this point!
  while(1)
    vTaskDelay(portMAX_DELAY);
}


/* Global system variables */
void systemStart()
{
  xSemaphoreGive(canStartMutex);
#ifndef DEBUG_EP2
  //watchdogInit();
#endif
}

void systemWaitStart(void)
{
  //This permits to guarantee that the system task is initialized before other
  //tasks waits for the start event.
  while(!isInit)
    vTaskDelay(2);

  xSemaphoreTake(canStartMutex, portMAX_DELAY);
  xSemaphoreGive(canStartMutex);
}

void systemSetCanFly(bool val)
{
  canFly = val;
}

bool systemCanFly(void)
{
  return canFly;
}

//TODO:watchdog disable now
// void vApplicationIdleHook( void )
// {
//   static uint32_t tickOfLatestWatchdogReset = M2T(0);

//   portTickType tickCount = xTaskGetTickCount();

//   if (tickCount - tickOfLatestWatchdogReset > M2T(WATCHDOG_RESET_PERIOD_MS))
//   {
//     tickOfLatestWatchdogReset = tickCount;
//     watchdogReset();
//   }

//   // Enter sleep mode. Does not work when debugging chip with SWD.
//   // Currently saves about 20mA STM32F405 current consumption (~30%).
// #ifndef DEBUG
//   { __asm volatile ("wfi"); }
// #endif
// }

PARAM_GROUP_START(system)
PARAM_ADD(PARAM_INT8, selftestPassed, &selftestPassed)
PARAM_GROUP_STOP(sytem)

/* Loggable variables */
LOG_GROUP_START(sys)
LOG_ADD(LOG_INT8, canfly, &canFly)
LOG_GROUP_STOP(sys)
