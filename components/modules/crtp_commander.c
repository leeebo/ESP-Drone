/**
 *
 * ESPlane Firmware
 *
 * Copyright 2019-2020  Espressif Systems (Shanghai)
 * Copyright (C) 2011-2017 Bitcraze AB
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
 * 注册用于飞行控制的协议端口，在commanderInit时，调用crtpCommanderInit
 *
 */
#include <stdbool.h>

#include "crtp_commander.h"
#include "cfassert.h"
#include "commander.h"
#include "crtp.h"
#define DEBUG_MODULE "CRTP_CMD"
#include "debug_cf.h"

static bool isInit;

static void commanderCrtpCB(CRTPPacket *pk);

void crtpCommanderInit(void) //在commanderInit时，调用crtpCommanderInit
{
    if (isInit) {
        return;
    }

    crtpInit();
    crtpRegisterPortCB(CRTP_PORT_SETPOINT, commanderCrtpCB);
    crtpRegisterPortCB(CRTP_PORT_SETPOINT_GENERIC, commanderCrtpCB);
    isInit = true;
}

enum crtpSetpointGenericChannel {
  SET_SETPOINT_CHANNEL = 0,
  META_COMMAND_CHANNEL = 1,
};
/* command step - receive  09 callback function of port CRTP_PORT_SETPOINT and CRTP_PORT_SETPOINT_GENERIC */
/* Channel 1 of the generic commander port is used for "meta-commands"
 * that alter the behavior of the commander itself, e.g. mode switching.
 * Although we use the generic commander port due to increasing pressure on the
 * 4-bit space of ports numbers, meta-commands that are unrelated to
 * streaming generic setpoint control modes are permitted.
 *
 * The packet format for meta-commands is:
 * +------+==========================+
 * | TYPE |     DATA                 |
 * +------+==========================+
 *
 * TYPE is an 8-bit value. The remainder of the data depends on the command.
 * The maximum data size is 29 bytes.
 */

/* To add a new packet:
 *   1 - Add a new type in the metaCommand_e enum.
 *   2 - Implement a decoder function with good documentation about the data
 *       structure and the intent of the packet.
 *   3 - Add the decoder function to the metaCommandDecoders array.
 *   4 - Create a new params group for your handler if necessary
 *   5 - Pull-request your change :-)
 */

/* ---===== 1 - metaCommand_e enum =====--- */
enum metaCommand_e {
  metaNotifySetpointsStop = 0,
  nMetaCommands,
};

typedef void (*metaCommandDecoder_t)(const void *data, size_t datalen);

/* ---===== 2 - Decoding functions =====--- */

/* notifySetpointsStop meta-command. See commander.h function
 * commanderNotifySetpointsStop() for description and motivation.
 */
struct notifySetpointsStopPacket {
  uint32_t remainValidMillisecs;
} __attribute__((packed));
void notifySetpointsStopDecoder(const void *data, size_t datalen)
{
  ASSERT(datalen == sizeof(struct notifySetpointsStopPacket));
  const struct notifySetpointsStopPacket *values = data;
  commanderNotifySetpointsStop(values->remainValidMillisecs);
}

 /* ---===== packetDecoders array =====--- */
const static metaCommandDecoder_t metaCommandDecoders[] = {
  [metaNotifySetpointsStop] = notifySetpointsStopDecoder,
};
static void commanderCrtpCB(CRTPPacket *pk)
{
    static setpoint_t setpoint;

  if(pk->port == CRTP_PORT_SETPOINT && pk->channel == 0) {
    crtpCommanderRpytDecodeSetpoint(&setpoint, pk);
    commanderSetSetpoint(&setpoint, COMMANDER_PRIORITY_CRTP);
  } else if (pk->port == CRTP_PORT_SETPOINT_GENERIC) {
    switch (pk->channel) {
    case SET_SETPOINT_CHANNEL:
      crtpCommanderGenericDecodeSetpoint(&setpoint, pk);
      commanderSetSetpoint(&setpoint, COMMANDER_PRIORITY_CRTP);
      break;
    case META_COMMAND_CHANNEL: {
        uint8_t metaCmd = pk->data[0];
        if (metaCmd < nMetaCommands && (metaCommandDecoders[metaCmd] != NULL)) {
          metaCommandDecoders[metaCmd](pk->data + 1, pk->size - 1);
        }
      }
      break;
    default:
      /* Do nothing */
      break;
    }
  }
}
