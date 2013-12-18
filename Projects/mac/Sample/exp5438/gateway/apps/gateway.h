#ifndef __GATEWAY_H
#define __GATEWAY_H

#ifdef __cplusplus
extern "C"
{
#endif

/**************************************************************************************************
 * INCLUDES
 **************************************************************************************************/
#include "hal_types.h"

#define __DEBUG

#define GW_PWR_MGMT_ENABLED      FALSE        /* Enable or Disable power saving */
#define GW_KEY_INT_ENABLED       TRUE         /* FALSE = Key Polling, TRUE  = Key interrupt */

#ifdef __DEBUG
  #define GW_SCAN_DURATION    6
#else
  #define GW_SCAN_DURATION    8
#endif

/* Event IDs */
#define GW_SEND_EVENT   0x0001
#define GW_POLL_EVENT   0x0002

/* Application State */
#define GW_IDLE_STATE     0x00
#define GW_SEND_STATE     0x01

/**************************************************************************************************
 * GLOBALS
 **************************************************************************************************/
extern uint8 GW_TaskId;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Mac Sample Application
 */
extern void GW_Init( uint8 task_id );

/*
 * Task Event Processor for the Mac Sample Application
 */
extern uint16 GW_ProcessEvent( uint8 task_id, uint16 events );

/*
 * Handle keys
 */
extern void GW_HandleKeys( uint8 keys, uint8 shift );

/*
 * Handle power saving
 */
extern void GW_PowerMgr (uint8 mode);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MACSAMPLEAPP_H */
