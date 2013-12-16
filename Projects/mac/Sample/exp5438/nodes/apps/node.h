#ifndef __NODE_H
#define __NODE_H

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/
#include "hal_types.h"

/**************************************************************************************************
 *                                        User's  Defines
 **************************************************************************************************/

#define NODE_MAC_CHANNEL           MAC_CHAN_11   /* Default channel - change it to desired channel */
#define NODE_DIRECT_MSG_ENABLED    FALSE         /* True if direct messaging is used, False if polling is used */
#define NODE_PACKET_LENGTH         20            /* Min = 4, Max = 102 */
#define NODE_PWR_MGMT_ENABLED      TRUE         /* Enable or Disable power saving */

#define NODE_KEY_INT_ENABLED       TRUE         /* FALSE = Key Polling, TRUE  = Key interrupt */

/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* Event IDs */
#define NODE_SEND_EVENT   0x0001
#define NODE_POLL_EVENT   0x0002

/* Application State */
#define NODE_IDLE_STATE     0x00
#define NODE_SEND_STATE     0x01

/**************************************************************************************************
 * GLOBALS
 **************************************************************************************************/
extern uint8 NODE_TaskId;

/****  FUNCTIONS ****/
extern void NODE_Init( uint8 task_id );
extern uint16 NODE_ProcessEvent( uint8 task_id, uint16 events );
extern void NODE_HandleKeys( uint8 keys, uint8 shift );
extern void NODE_PowerMgr (uint8 mode);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MACSAMPLEAPP_H */
