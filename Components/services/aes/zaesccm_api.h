/**
  @file  zaesccm_api.h
  @brief AES CCM service (OS dependent) interface

  <!--
  Revised:        $Date: 2012-08-09 08:46:45 -0700 (Thu, 09 Aug 2012) $
  Revision:       $Revision: 31169 $

  Copyright 2012-2013 Texas Instruments Incorporated. All rights reserved.

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
  PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
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
  -->
*/
#ifndef ZAESCCM_API_H
#define ZAESCCM_API_H

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Authenticates and encrypts a text using an MLE key.<br>
 * This function is thread-safe.
 *
 * @param  encrypt   set to TRUE to encrypt. FALSE to just authenticate without encryption.
 * @param  Mval      length of authentication field in octets
 * @param  N         13 byte nonce
 * @param  M         octet string 'm'
 * @param  len_m     length of M[] in octets
 * @param  A         octet string 'a'
 * @param  len_a     length of A[] in octets
 * @param  AesKey    Pointer to AES Key or Pointer to Key Expansion buffer
 * @param  MAC       MAC output buffer
 * @param  ccmLVal   ccm L value to be used
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
extern signed char zaesccmAuthEncrypt(unsigned char encrypt,
                                      unsigned char Mval, unsigned char *Nonce,
                                      unsigned char *M, unsigned short len_m,
                                      unsigned char *A, unsigned short len_a,
                                      unsigned char *AesKey,
                                      unsigned char *MAC, unsigned char ccmLVal);

/**
 * Decrypts and authenticates an encrypted text using an MLE key.
 * This function is thread-safe.
 *
 * @param  decrypt   set to TRUE to decrypt. Set to FALSE to authenticate without decryption.
 * @param  Mval      length of authentication field in octets
 * @param  N         13 byte nonce
 * @param  M         octet string 'm'
 * @param  len_m     length of M[] in octets
 * @param  A         octet string 'a'
 * @param  len_a     length of A[] in octets
 * @param  AesKey    Pointer to AES Key or Pointer to Key Expansion buffer.
 * @param  MAC       MAC output buffer
 * @param  ccmLVal   ccm L value to be used
 *
 * @return Zero when successful. Non-zero, otherwise.
 */
extern signed char zaesccmDecryptAuth(unsigned char decrypt,
                                      unsigned char Mval, unsigned char *Nonce,
                                      unsigned char *M, unsigned short len_m,
                                      unsigned char *A, unsigned short len_a,
                                      unsigned char *AesKey,
                                      unsigned char *MAC, unsigned char ccmLVal);

/**
 * Locks mutex used by AES CCM service module.
 * Note that this function is not provided by AES CCM service module but instead it has
 * to be implemented in order to link and use AES CCM service.
 */
extern void zaesccmMutexLock(void);

/**
 * Unlocks mutex used by AES CCM service module.
 * Note that this function is not provided by AES CCM service module but instead it has
 * to be implemented in order to link and use AES CCM service.
 */
extern void zaesccmMutexUnlock(void);

#ifdef __cplusplus
}
#endif

#endif /* ZAESCCM_API_H */
