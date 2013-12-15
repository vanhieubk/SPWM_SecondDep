/******************************************************************************
  Filename:       zaesccm_osal.c
  Revised:        $Date: 2012-10-02 13:23:38 -0700 (Tue, 02 Oct 2012) $
  Revision:       $Revision: 31673 $

  Description:    Support for CCM authentication using osal.

  Copyright 2013 Texas Instruments Incorporated. All rights reserved.

  IMPORTANT: Your use of this Software is limited to those specific rights
  granted under the terms of a software license agreement between the user
  who downloaded the software, his/her employer (which must be your employer)
  and Texas Instruments Incorporated (the "License"). You may not use this
  Software unless you agree to abide by the terms of the License. The License
  limits your use, and you acknowledge, that the Software may not be modified,
  copied or distributed unless embedded on a Texas Instruments microcontroller
  or used solely and exclusively in conjunction with a Texas Instruments radio
  frequency transceiver, which is integrated into your product. Other than for
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
******************************************************************************/

/******************************************************************************
 * INCLUDES
 */

#include "OSAL.h"
#include "hal_aes.h"
#include "hal_ccm.h"
#include "hal_assert.h"
#include "zaesccm_api.h"
#include "hal_mcu.h"

/******************************************************************************
 * MACROS
 */
#define AES_NO_RESOURCES              0x1A

/******************************************************************************
 * CONSTANTS
 */

/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */

/******************************************************************************
 * GLOBAL VARIABLES
 */

/* Flag to ensure AES not reentrant */
volatile uint8 aesInUse = FALSE;

/******************************************************************************
 * FUNCTION PROTOTYPES
 */

/* see zaesccm_api.h */
signed char zaesccmAuthEncrypt(unsigned char encrypt,
                               unsigned char Mval, unsigned char *Nonce,
                               unsigned char *M, unsigned short len_m,
                               unsigned char *A, unsigned short len_a,
                               unsigned char *AesKey,
                               unsigned char *MAC, unsigned char ccmLVal)
{

  unsigned char status;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION( intState );

 /* Make sure AES is not already in-use */
  if(aesInUse)
  {
    HAL_EXIT_CRITICAL_SECTION( intState );
    return AES_NO_RESOURCES;
  }

  aesInUse = TRUE;

  /* Initialize AES key */
  ssp_HW_KeyInit( AesKey );

  /* This is required for the MSP430 platform */
  pSspAesEncrypt = sspAesEncryptHW;

  status = SSP_CCM_Auth_Encrypt (encrypt, Mval, Nonce, M, len_m, A,
                        len_a, AesKey, MAC, ccmLVal);

  aesInUse = FALSE;
  HAL_EXIT_CRITICAL_SECTION( intState );
  return status;
}


/* see zaesccm_api.h */
signed char zaesccmDecryptAuth(unsigned char decrypt,
                               unsigned char Mval, unsigned char *Nonce,
                               unsigned char *M, unsigned short len_m,
                               unsigned char *A, unsigned short len_a,
                               unsigned char *AesKey,
                               unsigned char *MAC, unsigned char ccmLVal)
{
  unsigned char status;
  halIntState_t intState;

  HAL_ENTER_CRITICAL_SECTION( intState );

  /* Make sure AES is not already in-use */
  if(aesInUse)
  {
    HAL_EXIT_CRITICAL_SECTION( intState );
    return AES_NO_RESOURCES;
  }
  aesInUse = TRUE;

  /* Initialize AES key */
  ssp_HW_KeyInit( AesKey );

  /* This is required for the MSP430 platform */
  pSspAesEncrypt = sspAesEncryptHW;

  status = SSP_CCM_InvAuth_Decrypt(decrypt, Mval, Nonce, M, len_m, A, len_a,
                          AesKey, MAC, ccmLVal);
  aesInUse = FALSE;
  HAL_EXIT_CRITICAL_SECTION( intState );
  return (status);
}








