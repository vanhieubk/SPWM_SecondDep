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

#define GW_KEY_INT_ENABLED       TRUE         /* FALSE = Key Polling, TRUE  = Key interrupt */

#ifdef __DEBUG
  #define GW_SCAN_DURATION              6
  #define GW_DEFAULT_STICK_DURATION     6000
#else
  #define GW_SCAN_DURATION              8
  #define GW_DEFAULT_STICK_DURATION     60000
#endif

/* Event IDs */
#define GW_SEND_EVENT         0x0001
#define GW_PREP_INIT_EVENT    0x0002
#define GW_STICK_TIMER_EVENT  0x0004



extern uint8 GW_TaskId;

/**** FUNCTIONS  ******************/
extern void GW_Init( uint8 task_id );
extern uint16 GW_ProcessEvent( uint8 task_id, uint16 events );
extern void GW_HandleKeys( uint8 keys, uint8 shift );
extern void GW_PowerMgr (uint8 mode);



#ifdef __cplusplus
}
#endif

#endif /* MACSAMPLEAPP_H */
