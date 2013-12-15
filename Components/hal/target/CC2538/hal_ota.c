/******************************************************************************
  Filename:       hal_ota.c
  Revised:        $Date: 2012-06-13 16:42:14 -0700 (Wed, 13 Jun 2012) $
  Revision:       $Revision: 30737 $

  Description:    HAL support for OTA for the CC2538.

  Copyright 2012 Texas Instruments Incorporated. All rights reserved.

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
******************************************************************************/

/******************************************************************************
 * INCLUDES
 */

#include <string.h>

#include "armcm3flashutil.h"
#include "comdef.h"
#include "hal_board_cfg.h"
#include "hal_ota.h"
#include "hal_types.h"

#include "ota_common.h"

/******************************************************************************
 * TYPEDEFS
 */

typedef struct
{
  uint32 crc[2];
  uint32 programSize;  // Note how this assumes the OTA_Preamble is immediately adjacent to OTA_CRC.
} OTA_CrcControl_t;
static_assert((sizeof(OTA_CrcControl_t) == 12), "Need to PACK the OTA_CrcControl_t");

typedef struct {
  uint32 imageCRC[2];
  uint32 nvicJump[2];
} ota_ledger_t;
static_assert((sizeof(ota_ledger_t) == 16), "Need to PACK the ota_ledger_t");

/******************************************************************************
 * CONSTANTS
 */

#define HAL_OTA_LEDGER_PAGE        254
#define HAL_OTA_LEDGER_PAGE_ADDR  (FLASH_BASE + (HAL_OTA_LEDGER_PAGE * HAL_FLASH_PAGE_SIZE))

static const ota_ledger_t LedgerPageSignature = {
  0x01234567,
  0x89ABCDEF,
  0x02468ACE,
  0x13579BDF
};

//*****************************************************************************
//
// Special setup for access to a HAPI function.
//
//*****************************************************************************
typedef long (* volatile HAPI_CRC32_T)(unsigned char *, unsigned long);
#define HAPI_CRC32_FUNC_PTR     0x00000048
#define P_HAPI_CRC32            ((HAPI_CRC32_T *)HAPI_CRC32_FUNC_PTR)
#define CalcCrc32(a,b)          (*P_HAPI_CRC32)(a,b)

/******************************************************************************
 * LOCAL VARIABLES
 */

const otaCrc_t OTA_CRC @ ".crc" =
{
  0x20120613,  // CRC
  0x20120613   // CRC Shadow
};
#pragma required=OTA_CRC

const preamble_t OTA_Preamble @ ".preamble" =
{
  HAL_OTA_RC_MAX,       // Default program length of max if not using post-processing tool.
  OTA_MANUFACTURER_ID,  // Manufacturer ID
  OTA_TYPE_ID,          // Image Type
#if (!defined HAL_IMG_AREA) || (HAL_IMG_AREA == 0)  // Use even-numbered version numbers.
#if defined BUILD_ALL_DEVICES
  0xC0000000,           // Image Version prefixed 11xx for all devices build.
#elif defined ZDO_COORDINATOR
  0x80000000,           // Image Version prefixed 10xx for ZC build.
#elif defined RTR_NWK
  0x40000000,           // Image Version prefixed 01xx for ZR build.
#else
  0x00000000,           // Image Version prefixed 00xx for ZED build.
#endif
#else  // IMAGE_AREA-B, so OR 0x01 with version LSB for a visual clue when using OTA Dongle.
#if defined BUILD_ALL_DEVICES
  0xC0000001,           // Image Version prefixed 11xx for all devices build.
#elif defined ZDO_COORDINATOR
  0x80000001,           // Image Version prefixed 10xx for ZC build.
#elif defined RTR_NWK
  0x40000001,           // Image Version prefixed 01xx for ZR build.
#else
  0x00000001,           // Image Version prefixed 00xx for ZED build.
#endif
#endif
};
#pragma required=OTA_Preamble

static OTA_CrcControl_t crcControl;

/******************************************************************************
 * LOCAL FUNCTIONS
 */

static ota_ledger_t * getCurrLedgerEntry(void);
static void resetLedgerPage(void);
static void setCurrLedgerEntry(ota_ledger_t *pNewLedger);

/******************************************************************************
 * @fn      HalOTAChkDL
 *
 * @brief   Run the CRC16 Polynomial calculation over the DL image.
 *
 * @param   None
 *
 * @return  SUCCESS or FAILURE.
 */
uint8 HalOTAChkDL(uint32 dlImagePreambleOffset)
{
  (void)dlImagePreambleOffset;  // Intentionally unreferenced parameter.

  // Get the CRC Control structure
  HalOTARead(HAL_OTA_CRC_OSET, (uint8 *)&crcControl, sizeof(crcControl), HAL_OTA_DL);

  if ((crcControl.programSize > HalOTAAvail()) || (crcControl.programSize == 0) ||
      (crcControl.crc[0] == 0) || (crcControl.crc[0] == 0xFFFFFFFF))
  {
    return FAILURE;
  }

  if (crcControl.crc[0] == crcControl.crc[1])
  {
    return SUCCESS;
  }
  else if (crcControl.crc[1] != 0xFFFFFFFF)
  {
    return FAILURE;
  }

  // Save current cache mode since the HAPI function will change it.
  uint32 ulCurrentCacheMode = FlashCacheModeGet();

  // Run the CRC calculation over the downloaded image, not including the CRC.
  crcControl.crc[1] = CalcCrc32((uint8*)(HAL_OTA_DL_ADDR+PREAMBLE_OFFSET),crcControl.programSize-8);

  FlashCacheModeSet(ulCurrentCacheMode);  // Restore cache mode.

  crcControl.crc[0] = 0xFFFFFFFF;
  HalOTAWrite(HAL_OTA_CRC_OSET, crcControl.crc, sizeof(crcControl.crc), HAL_OTA_DL);
  HalOTARead(HAL_OTA_CRC_OSET, crcControl.crc, sizeof(crcControl.crc), HAL_OTA_DL);

  return (crcControl.crc[0] == crcControl.crc[1]) ? SUCCESS : FAILURE;
}

/******************************************************************************
 * @fn      HalOTAInit
 *
 * @brief   Ensure that this image is setup to run.
 *
 * @param   None.
 *
 * @return  None.
 */
void HalOTAInit(void)
{
  ota_ledger_t *pLedger = getCurrLedgerEntry();

  if (pLedger->imageCRC[0] == 0xFFFFFFFF)
  {
    ota_ledger_t ledger;

    HalOTARead(HAL_OTA_CRC_OSET, ledger.imageCRC, sizeof(((ota_ledger_t*)0)->imageCRC), HAL_OTA_RC);
    HalOTARead(HAL_NVIC_OSET,    ledger.nvicJump, sizeof(((ota_ledger_t*)0)->nvicJump), HAL_OTA_RC);

    setCurrLedgerEntry(&ledger);
  }
}

/******************************************************************************
 * @fn      HalOTAInvRC
 *
 * @brief   Invalidate the active image so that the boot manager will run the DL image
 *          on the next reset.
 *
 * @param   None.
 *
 * @return  None.
 */
void HalOTAInvRC(void)
{
  ota_ledger_t ledger;

  HalOTARead(HAL_OTA_CRC_OSET, ledger.imageCRC, sizeof(((ota_ledger_t *)0)->imageCRC), HAL_OTA_DL);
  HalOTARead(HAL_NVIC_OSET,    ledger.nvicJump, sizeof(((ota_ledger_t *)0)->nvicJump), HAL_OTA_DL);

  setCurrLedgerEntry(&ledger);
}

/******************************************************************************
 * @fn      HalOTARead
 *
 * @brief   Read from the storage medium according to image type.
 *
 * @param   oset - Offset into the monolithic image.
 * @param   pBuf - Pointer to the buffer in which to copy the bytes read.
 * @param   len - Number of bytes to read.
 * @param   type - Which image: HAL_OTA_RC or HAL_OTA_DL.
 *
 * @return  None.
 */
void HalOTARead(uint32 oset, uint8 *pBuf, uint16 len, image_t type)
{
  oset += (HAL_OTA_RC == type) ? HAL_OTA_RC_ADDR : HAL_OTA_DL_ADDR;

  memcpy(pBuf, (uint8 *)oset, len);
}

/******************************************************************************
 * @fn      HalOTAWrite
 *
 * @brief   Write to the storage medium according to the image type.
 *
 *  NOTE:   Destructive write on page boundary! When writing to the first flash word
 *          of a page boundary, the page is erased without saving/restoring the bytes not written.
 *          Writes anywhere else on a page assume that the location written to has been erased.
 *
 * @param   oset - Offset into the monolithic image, aligned to HAL_FLASH_WORD_SIZE.
 * @param   pBuf - Pointer to the buffer in from which to write.
 * @param   len - Number of bytes to write. If not an even multiple of HAL_FLASH_WORD_SIZE,
 *                remainder bytes are overwritten with garbage.
 * @param   type - Which image: HAL_OTA_RC or HAL_OTA_DL.
 *
 * @return  None.
 */
void HalOTAWrite(uint32 oset, uint8 *pBuf, uint16 len, image_t type)
{
  oset += (HAL_OTA_RC == type) ? HAL_OTA_RC_ADDR : HAL_OTA_DL_ADDR;

  if ((oset % HAL_FLASH_PAGE_SIZE) == 0)
  {
    flashErasePage((uint8 *)oset);
  }

  flashWrite((uint8 *)oset, len, pBuf);
}

/******************************************************************************
 * @fn      HalOTAAvail
 *
 * @brief   Determine the space available for downloading an image.
 *
 * @param   None.
 *
 * @return  Number of bytes available for storing an OTA image.
 */
uint32 HalOTAAvail(void)
{
  return HAL_OTA_DL_MAX;
}

/******************************************************************************
 * @fn      getCurrLedgerEntry
 *
 * @brief   Find the current Ledger Entry being used by the IBM.
 *
 * @param   None.
 *
 * @return  Address of the current Ledger Entry being used by the IBM.
 */
static ota_ledger_t * getCurrLedgerEntry(void)
{
  ota_ledger_t *pLedger = (ota_ledger_t*)HAL_OTA_LEDGER_PAGE_ADDR;
  int ledgerCnt = 0;

  if (!memcmp(pLedger, &LedgerPageSignature, sizeof(ota_ledger_t)))
  {
    for (ledgerCnt = 1, pLedger++; ledgerCnt < (HAL_FLASH_PAGE_SIZE / sizeof(ota_ledger_t));
         ledgerCnt++, pLedger++)
    {
      if (pLedger->imageCRC[0] == 0xFFFFFFFF)  // Not expected except first 2-step programming.
      {
        break;
      }

      if ((pLedger->imageCRC[0] != 0) && (pLedger->imageCRC[0] == pLedger->imageCRC[1]))
      {
        // Sanity check NVIC entries.
        if ((pLedger->nvicJump[0] > 0x20004000) &&
            (pLedger->nvicJump[0] < 0x27007FFF) &&
            (pLedger->nvicJump[1] > 0x00200000) &&
            (pLedger->nvicJump[1] < 0x0027EFFF))
        {
          return pLedger;
        }
      }
    }
  }

  // The following test is not expected to ever fail, since one ledger page allows for 63-126
  // iterations of an OTA. Thus if implementing a dual stack and very many ping-pong between
  // images is expected, code should be added to account for the possibility of a reset during
  // the resetLedgerPage() operation which could leave the boot manager with no valid image.
  if ((ledgerCnt == 0) || (ledgerCnt >= (HAL_FLASH_PAGE_SIZE / sizeof(ota_ledger_t))))
  {
    resetLedgerPage();
    return (ota_ledger_t *)(HAL_OTA_LEDGER_PAGE_ADDR + sizeof(ota_ledger_t));
  }

  return pLedger;
}

/******************************************************************************
 * @fn      resetLedgerPage
 *
 * @brief   Erase the Ledger Page and re-write the signature.
 *          NOTE: This function does not account for the possibility of a reset during this
 *          ledger page reset operation which could leave the boot manager with no valid image.
 *
 * @param   None.
 *
 * @return  None.
 */
static void resetLedgerPage(void)
{
  ota_ledger_t *pLedger = (ota_ledger_t*)HAL_OTA_LEDGER_PAGE_ADDR;
  ota_ledger_t ledgerPageSignature;

  memcpy(&ledgerPageSignature, &LedgerPageSignature, sizeof(ota_ledger_t));

  while (1)
  {
    flashErasePage((uint8 *)HAL_OTA_LEDGER_PAGE_ADDR);
    flashWrite((uint8 *)HAL_OTA_LEDGER_PAGE_ADDR, sizeof(ota_ledger_t), &ledgerPageSignature);

    if (!memcmp(&ledgerPageSignature, (void *)HAL_OTA_LEDGER_PAGE_ADDR, sizeof(ota_ledger_t)))
    {
      break;
    }
  }
}

/******************************************************************************
 * @fn      setCurrLedgerEntry
 *
 * @brief   Set the current Ledger Entry to be used by the IBM.
 *
 * @param   pLedger - values to set the current ledger to.
 *
 * @return  None.
 */
static void setCurrLedgerEntry(ota_ledger_t *pNewLedger)
{
  while (1)
  {
    ota_ledger_t tmpLedger = {
      { 0xFFFFFFFF, 0xFFFFFFFF },
      { 0xFFFFFFFF, 0xFFFFFFFF }
    };

    ota_ledger_t *pCurrLedger = getCurrLedgerEntry();
    ota_ledger_t *pNextLedger = pCurrLedger + 1;

    if (!memcmp(pCurrLedger, pNewLedger, sizeof(ota_ledger_t)))
    {
      return;
    }

    if (memcmp(pNextLedger, &tmpLedger, sizeof(ota_ledger_t)))
    {
      resetLedgerPage();
    }
    else
    {
      // Write the next ledger except for the CRC-shadow.
      memcpy(&tmpLedger, pNewLedger, sizeof(ota_ledger_t));
      tmpLedger.imageCRC[1] = 0xFFFFFFFF;
      flashWrite(pNextLedger, sizeof(ota_ledger_t), &tmpLedger);

      // If the next ledger is valid except for the CRC-shadow, write the CRC-shadow equal.
      if (!memcmp(pNextLedger, &tmpLedger, sizeof(ota_ledger_t)))
      {
        memset(&tmpLedger, 0xFF, sizeof(ota_ledger_t));
        tmpLedger.imageCRC[1] = pNewLedger->imageCRC[1];
        flashWrite(pNextLedger, sizeof(ota_ledger_t), &tmpLedger);
      }

      // If the next ledger is completely valid, invalidate the current ledger.
      if (!memcmp(pNextLedger, pNewLedger, sizeof(ota_ledger_t)))
      {
        memset(&tmpLedger, 0xFF, sizeof(ota_ledger_t));
        tmpLedger.imageCRC[0] = pCurrLedger->imageCRC[0] ^ 0xFFFFFFFF;
        flashWrite(pCurrLedger, sizeof(ota_ledger_t), &tmpLedger);
      }
    }
  }
}

/******************************************************************************
*/
