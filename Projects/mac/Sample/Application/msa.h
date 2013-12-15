/**************************************************************************************************
  Filename:       msa.h
  Revised:        $Date: 2013-04-18 10:57:13 -0700 (Thu, 18 Apr 2013) $
  Revision:       $Revision: 33955 $

  Description:    This file contains the the Mac Sample Application protypes and definitions


  Copyright 2006-2007 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License").  You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product.  Other than for
  the foregoing purpose, you may not use, reproduce, copy, prepare derivative
  works of, modify, distribute, perform, display or sell this Software and/or
  its documentation for any purpose.

  YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
  PROVIDED “AS IS” WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
  NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
  TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
  NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
  LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
  INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
  OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
  OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
  (INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

  Should you have any questions regarding your right to use this Software,
  contact Texas Instruments Incorporated at www.TI.com.
**************************************************************************************************/

#ifndef MACSAMPLEAPP_H
#define MACSAMPLEAPP_H

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

#define MSA_MAC_CHANNEL           MAC_CHAN_11   /* Default channel - change it to desired channel */
#define MSA_WAIT_PERIOD           100           /* Time between each packet - Lower = faster sending rate */

#define MSA_PAN_ID                0x11CC        /* PAN ID */
#define MSA_COORD_SHORT_ADDR      0xAABB        /* Coordinator short address */

#define MSA_DIRECT_MSG_ENABLED    TRUE          /* True if direct messaging is used, False if polling is used */

#define MSA_MAC_BEACON_ORDER      15            /* Setting beacon order to 15 will disable the beacon */
#define MSA_MAC_SUPERFRAME_ORDER  15            /* Setting superframe order to 15 will disable the superframe */

#define MSA_PACKET_LENGTH         20            /* Min = 4, Max = 102 */

#define MSA_PWR_MGMT_ENABLED      FALSE         /* Enable or Disable power saving */

#define MSA_KEY_INT_ENABLED       FALSE         /*
                                                 * FALSE = Key Polling
                                                 * TRUE  = Key interrupt
                                                 *
                                                 * Notes: Key interrupt will not work well with 2430 EB because
                                                 *        all the operations using up/down/left/right switch will
                                                 *        no longer work. Normally S1 + up/down/left/right is used
                                                 *        to invoke the switches but on the 2430 EB board,  the
                                                 *        GPIO for S1 is used by the LCD.
                                                 */

#if (MSA_MAC_SUPERFRAME_ORDER > MSA_MAC_BEACON_ORDER)
#error "ERROR! Superframe order cannot be greater than beacon order."
#endif

#if ((MSA_MAC_SUPERFRAME_ORDER != 15) || (MSA_MAC_BEACON_ORDER != 15)) && (MSA_DIRECT_MSG_ENABLED == FALSE)
#error "ERROR! Cannot run beacon enabled on a polling device"
#endif

#if (MSA_PACKET_LENGTH < 4) || (MSA_PACKET_LENGTH > 102)
#error "ERROR! Packet length has to be between 4 and 102"
#endif

/**************************************************************************************************
 * CONSTANTS
 **************************************************************************************************/

/* Event IDs */
#define MSA_SEND_EVENT   0x0001
#define MSA_POLL_EVENT   0x0002

/* Application State */
#define MSA_IDLE_STATE     0x00
#define MSA_SEND_STATE     0x01

/**************************************************************************************************
 * GLOBALS
 **************************************************************************************************/
extern uint8 MSA_TaskId;

/*********************************************************************
 * FUNCTIONS
 */

/*
 * Task Initialization for the Mac Sample Application
 */
extern void MSA_Init( uint8 task_id );

/*
 * Task Event Processor for the Mac Sample Application
 */
extern uint16 MSA_ProcessEvent( uint8 task_id, uint16 events );

/*
 * Handle keys
 */
extern void MSA_HandleKeys( uint8 keys, uint8 shift );

/*
 * Handle power saving
 */
extern void MSA_PowerMgr (uint8 mode);

/*********************************************************************
*********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* MACSAMPLEAPP_H */
