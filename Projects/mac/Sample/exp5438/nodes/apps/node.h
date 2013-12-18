#ifndef __NODE_H
#define __NODE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"


/****  DEFINEs  ****/
#define NODE_PWR_MGMT_ENABLED      TRUE         /* Enable or Disable power saving */

#define NODE_KEY_INT_ENABLED       TRUE         /* FALSE = Key Polling, TRUE  = Key interrupt */

#ifdef __DEBUG
  #define NODE_DEFAULT_SCAN_TIME_OUT    3
  #define NODE_DEFAULT_RESCAN_WAIT_TIME 6000
  #define NODE_DEFAULT_NUM_RESCAN_TRY   3
  #define NODE_DEFAULT_STICK_DURATION   1000
  #define NODE_DEFAULT_PREPARING_DELTA  5
  #define NODE_DEFAULT_SENSING_TIME     2000
  #define NODE_DEFAULT_SEND_ALIVE_TIME  2 /* send alive time, must < sensing time */
#else
  #define NODE_DEFAULT_SCAN_TIME_OUT    4
  #define NODE_DEFAULT_NUM_RESCAN_TRY   3
  #define NODE_DEFAULT_RESCAN_WAIT_TIME 60000
  #define NODE_DEFAULT_STICK_DURATION   60000
  #define NODE_DEFAULT_PREPARING_DELTA  2
  #define NODE_DEFAULT_SENSING_TIME     30
  #define NODE_DEFAULT_SEND_ALIVE_TIME  5 /* send alive time, must < sensing time */
#endif
/**** Event IDs ****/
#define NODE_STICK_TIMER_EVENT          0x0001
#define NODE_SEND_EVENT                 0x0002
#define NODE_RESCAN_EVENT               0x0004
#define NODE_PREP_INIT_EVENT            0x0008

/**** Application State ****/
#define NODE_IDLE_STATE     0x00
#define NODE_SEND_STATE     0x01

/****  GLOBAL VARIABLEs  ****/
extern uint8 NODE_TaskId;

/****  FUNCTIONS ****/
extern void NODE_Init( uint8 task_id );
extern uint16 NODE_ProcessEvent( uint8 task_id, uint16 events );
extern void NODE_HandleKeys( uint8 keys, uint8 shift );
extern void NODE_PowerMgr (uint8 mode);

#ifdef __cplusplus
}
#endif

#endif /* __NODE_H */
