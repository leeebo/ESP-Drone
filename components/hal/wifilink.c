/*
*
 * ESPlane Firmware
 * 
 * Copyright 2019-2020  Espressif Systems (Shanghai) 
 * Copyright (C) 2012 BitCraze AB
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
 * wifilink.c: ESP32 Wi-Fi implementation of the CRTP link
 */

#include <stdbool.h>
#include <string.h>
#define DEBUG_MODULE "WIFILINK"
#include "config.h"
#include "wifilink.h"
#include "crtp.h"
#include "configblock.h"
#include "ledseq.h"
#include "pm.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "queuemonitor.h"
#include "freertos/semphr.h"

#include "wifi_esp32.h"
#include "stm32_legacy.h"
#include "debug_cf.h"

#define WIFI_ACTIVITY_TIMEOUT_MS (1000)

static bool isInit = false;
static xQueueHandle crtpPacketDelivery;
static uint8_t sendBuffer[64];

static UDPPacket wifiIn;
static CRTPPacket p;
static bool isOldVersionApp =false;
static uint32_t lastPacketTick;

static int wifilinkSendPacket(CRTPPacket *p);
static int wifilinkSetEnable(bool enable);
static int wifilinkReceiveCRTPPacket(CRTPPacket *p);

static float rch,pch,ych;
static uint16_t tch;

static bool detectOldVersionApp(UDPPacket *in){

  if ((in->data)[0]!=0x00&&in->size == 11)//12-1
  {
    if ((in->data)[0]==0x80&&(in->data)[9]==0x00&&(in->data)[10]==0x00&&(in->data)[11]==0x00)
    {
      isOldVersionApp = true;
      return true;
    }
    
  }
  isOldVersionApp = false;
  return false;
}

static bool wifilinkIsConnected(void) {
  return (xTaskGetTickCount() - lastPacketTick) < M2T(WIFI_ACTIVITY_TIMEOUT_MS);
}

static struct crtpLinkOperations wifilinkOp =
{
  .setEnable         = wifilinkSetEnable,
  .sendPacket        = wifilinkSendPacket,
  .receivePacket     = wifilinkReceiveCRTPPacket,
  .isConnected       = wifilinkIsConnected,
};

/* Radio task handles the CRTP packet transfers as well as the radio link
 * specific communications (eg. Scann and ID ports, communication error handling
 * and so much other cool things that I don't have time for it ...)
 */

static void wifilinkTask(void *param)
{
  while(1)
  {
    // Fetch a wifi packet off the queue
    wifiGetDataBlocking(&wifiIn);//command receive step 03
    lastPacketTick = xTaskGetTickCount();
#ifdef FIT_EP10_APP
    if(detectOldVersionApp(&wifiIn)){

      rch  = (1.0)*(float)(((((uint16_t)wifiIn.data[1]<<8)+(uint16_t)wifiIn.data[2])-296)*15.0/150.0); //-15~+15
      pch  = (-1.0)*(float)(((((uint16_t)wifiIn.data[3]<<8)+(uint16_t)wifiIn.data[4])-296)*15.0/150.0);//-15~+15
      tch  = (((uint16_t)wifiIn.data[5]<<8)+(uint16_t)wifiIn.data[6])*59000.0/600.0;
      ych  = (float)(((((uint16_t)wifiIn.data[7]<<8)+(uint16_t)wifiIn.data[8])-296)*15.0/150.0);//-15~+15
      p.size = wifiIn.size + 1 ; //把cksum字节加上
      p.header = CRTP_HEADER(CRTP_PORT_SETPOINT, 0x00); //重新定义头
      
      memcpy(&p.data[0], &rch, 4);
      memcpy(&p.data[4], &pch, 4);  
      memcpy(&p.data[8], &ych, 4); 
      memcpy(&p.data[12],&tch, 2);   

    }else
#endif
    {
      p.size = wifiIn.size - 1;//CRTP中的size是去掉head的大小
      memcpy(&p.raw, wifiIn.data, wifiIn.size);  //command receive step 04
    }
    
    // This queuing will copy a CRTP packet size from wifiIn
    xQueueSend(crtpPacketDelivery, &p, 0) ; //command receive step 05
  }

}

static int wifilinkReceiveCRTPPacket(CRTPPacket *p)
{
  if (xQueueReceive(crtpPacketDelivery, p, M2T(100)) == pdTRUE) //command receive step 06
  {
    ledseqRun(LINK_LED, seq_linkup);
    DEBUG_PRINTD("3.wifilinkReceiveCRTPPacket got data size = %d",p->size);
    return 0;
  }
  return -1;
}

static int wifilinkSendPacket(CRTPPacket *p)
{
  int dataSize;

  ASSERT(p->size < SYSLINK_MTU);

  sendBuffer[0] = p->header;

  if (p->size <= CRTP_MAX_DATA_SIZE)
  {
    memcpy(&sendBuffer[1], p->data, p->size);
  }
  dataSize = p->size + 1;


  /*ledseqRun(LINK_DOWN_LED, seq_linkup);*/

  return wifiSendData(dataSize, sendBuffer);
}

static int wifilinkSetEnable(bool enable)
{
  return 0;
}

/*
 * Public functions
 */

void wifilinkInit()
{
  if(isInit)
    return;

  //TODO: Initialize the wifi peripheral
  //wifiInit();
  
  crtpPacketDelivery = xQueueCreate(16, sizeof(CRTPPacket));
  DEBUG_QUEUE_MONITOR_REGISTER(crtpPacketDelivery);

  xTaskCreate(wifilinkTask, WIFILINK_TASK_NAME,WIFILINK_TASK_STACKSIZE, NULL, WIFILINK_TASK_PRI, NULL);

  isInit = true;
}

bool wifilinkTest()
{
  return isInit;
}

struct crtpLinkOperations * wifilinkGetLink()
{
  return &wifilinkOp;
}

