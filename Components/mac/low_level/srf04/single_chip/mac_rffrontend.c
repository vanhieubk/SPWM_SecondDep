/**************************************************************************************************
  Filename:       mac_rffrontend.c
  Revised:        $Date: 2012-12-07 13:18:43 -0800 (Fri, 07 Dec 2012) $
  Revision:       $Revision: 32494 $

  Description:    RF frontend configuration module


  Copyright 2009-2012 Texas Instruments Incorporated. All rights reserved.

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

/* ------------------------------------------------------------------------------------------------
 *                                             Includes
 * ------------------------------------------------------------------------------------------------
 */
#include "hal_types.h"
#include "hal_board_cfg.h"
#include "hal_assert.h"
#include "mac_api.h"
#include "mac_radio_defs.h"


/* ------------------------------------------------------------------------------------------------
 *                                        Global Constants
 * ------------------------------------------------------------------------------------------------
 */

/* The following table index definitions are specific to
 * a build with both MAC_RUNTIME_CC2590 and MAC_RUNTIME_CC2591
 * compile flags defined. */
#define MAC_CC2591_TX_PWR_TABLE_IDX   1
#define MAC_CC2590_TX_PWR_TABLE_IDX   2
#define MAC_CC2591_HGM_RSSI_ADJ_IDX   1
#define MAC_CC2591_LGM_RSSI_ADJ_IDX   2
#define MAC_CC2590_HGM_RSSI_ADJ_IDX   3
#define MAC_CC2590_LGM_RSSI_ADJ_IDX   4

/* Set of indices when only one of either MAC_RUNTIME_CC2590
 * or MAC_RUNTIME_CC2591 was configured in the build.
 */
#define MAC_CC259X_TX_PWR_TABLE_IDX 1
#define MAC_CC259X_HGM_RSSI_ADJ_IDX 1
#define MAC_CC259X_LGM_RSSI_ADJ_IDX 2

/* Set of indices when only one one of either HAL_PA_LNA
 * or HAL_PA_LNA_CC2590 was configured in the build
 */
#define MAC_PA_LNA_HGM_RSSI_ADJ_IDX 0
#define MAC_PA_LNA_LGM_RSSI_ADJ_IDX 1

/* ------------------------------------------------------------------------------------------------
 *                                           MACROS
 * ------------------------------------------------------------------------------------------------
 */

/* ------------------------------------------------------------------------------------------------
 *                                       Function Prototypes
 * ------------------------------------------------------------------------------------------------
 */
void MAC_RfFrontendSetup(void);

/**************************************************************************************************
 * @fn          MAC_RfFrontendSetup
 *
 * @brief       Setup RF frontend.
 *
 * @param       none
 *
 * @return      none
 **************************************************************************************************
 */
void MAC_RfFrontendSetup(void)
{
  /* CC2591 HGM pin control configuration.
   *   P0_7 -> HGM
   */
  HAL_PA_LNA_RX_HGM();

  /* Raises the CCA threshold to about -70 dBm input level.
   */
  CCACTRL0 = CCA_THR_HGM;

  /* Select power register value table and RSSI adjustment value table */
  #if (defined MAC_RUNTIME_CC2591 && defined MAC_RUNTIME_CC2590)
  /* Select power register value table and RSSI adjustment value table.
   * Note that this file selected CC2591. The file has to be modified
   * if the target board has CC2590 instead.
   */
  MAC_SetRadioRegTable(MAC_CC2591_TX_PWR_TABLE_IDX, MAC_CC2591_HGM_RSSI_ADJ_IDX);

  #elif defined (MAC_RUNTIME_CC2591) || defined (MAC_RUNTIME_CC2590)
  /* Select power register value table and RSSI adjustment value table */
  MAC_SetRadioRegTable(MAC_CC259X_TX_PWR_TABLE_IDX, MAC_CC259X_HGM_RSSI_ADJ_IDX);

  #elif defined (HAL_PA_LNA) || defined (HAL_PA_LNA_CC2590)
  /* No need to do anything here because by default macRadioDefsRefTableId = 0 hence,
   * automatically setup for HGM. However if you want LGM modify this file and call
   * MAC_SetRadioRegTable(0,  MAC_PA_LNA_LGM_RSSI_ADJ_IDX);
   */
  #endif
}

/**************************************************************************************************
 */
