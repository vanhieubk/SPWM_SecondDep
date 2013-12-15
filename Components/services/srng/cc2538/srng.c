/******************************************************************************
  Filename:       srng.c
  Revised:        $Date: 2012-05-10 13:33:10 -0700 (Thu, 10 May 2012) $
  Revision:       $Revision: 30528 $

  Description:   Secure Random Number Generator for cc2538.

  Copyright 2011-2012 Texas Instruments Incorporated. All rights reserved.

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
#include "aes.h"
#include "hal_types.h"
#include "hal_aes.h"
#include "srng.h"

/******************************************************************************
 * MACROS
 */
#define MAX_32BIT_VAL          0xFFFFFFFFUL
#define MIN_32BIT_VAL          0x00000000UL
#define SRNGV_CHECK_VAL        0xFFFFFFFCUL

/******************************************************************************
 * CONSTANTS
 */
static uint32 const add[4] = { 0x00000001,
                        0x00000000,
                        0x00000000,
                        0x00000000};
static uint64 const reseed_counter = 0x1000000000000;
/******************************************************************************
 * TYPEDEFS
 */

/******************************************************************************
 * LOCAL VARIABLES
 */
static uint32 srng_key[4]; /* SRNG Key */
static uint32 srng_v[4];   /* SRNG V*/
static uint64 reseed_interval = 0;

/******************************************************************************
 * GLOBAL VARIABLES
 */
 
/******************************************************************************
 * FUNCTION PROTOTYPES
 */
typedef struct  { 
    uint32 word0; /* LSBs */
    uint32 word1;
    uint32 word2;
    uint32 word3; /* MSBs */
}uint128;

/*******************************************************************************
 * @fn          add128
 *
 * @brief     add 128 bit numbers
 *
 * input parameters
 *
 * @param a pointer to the first 128bit value
 * @param b pointer to the second 128bit value
 * @param out pointer to output value
 *
 * output parameters
 *
 * @param output address will contain output value 
 *               by adding a and b
 *
 * @return Overflow error
 *******************************************************************************
 */
uint32 add128(uint128 *out, uint128 *a, uint128 *b)
{
    /* use an intermediate large enough to hold the result
       of adding two 16 bit numbers together. */
    uint64 partial;
    uint128 result;

    /* add the chunks together, least significant bit first */
    partial = (uint64)a->word0 + b->word0;

    /* extract the low 16 bits of the sum and store it */
    result.word0 = partial & MAX_32BIT_VAL;

    /* add the overflow to the next chunk of 32 */
    partial = ((partial >> 32) + ((uint64)a->word1 + b->word1));
    /* keep doing this for the remaining bits */
    result.word1 = partial & MAX_32BIT_VAL;
    partial = ((partial >> 32) + ((uint64)a->word2 + b->word2));
    result.word2 = partial & MAX_32BIT_VAL;
    partial = ((partial >> 32) + ((uint64)a->word3 + b->word3));
    result.word3 = partial & MAX_32BIT_VAL;
    
    
    out->word0 = result.word0;
    out->word1 = result.word1;
    out->word2 = result.word2;
    out->word3 = result.word3;
    
    /* if the result overflowed, return nonzero */
    return partial >> 32;
}

/*******************************************************************************
 * @fn          srng_init
 *
 * @brief     Initialize SRNG key and V  
 *
 * input parameters
 *
 * @param entropy/random seed is passed in
 *
 * output parameters
 *
 * None
 *
 * @return None
 *******************************************************************************
 */
void srng_init(uint8 * entropy)
{
  uint8 i;
  uint32 srng_tmp1[RNG_BLOCK_LENGTH_32BITS];
  uint32 srng_tmp2[RNG_BLOCK_LENGTH_32BITS];
  uint8 *psrng_v = (uint8 *)srng_v;
  uint8 *psrng_key = (uint8 *)srng_key;
  uint8 *psrng_tmp1 = (uint8 *)srng_tmp1;
  uint8 *psrng_tmp2 = (uint8 *)srng_tmp2;
  
  reseed_interval = 1;
  
  for( i=0; i<RNG_BLOCK_LENGTH_32BITS; i++)
  {
    srng_key[i] = MIN_32BIT_VAL;
    srng_v[i] = MIN_32BIT_VAL;
  }
  
  add128((uint128 *)srng_v,(uint128 *)srng_v, (uint128 *)add);
  
  AESLoadKey( (uint8 *)srng_key, KEY_AREA_5);
  sspAesEncryptHW_keylocation((uint8 *)srng_v, (uint8*) srng_tmp1, KEY_AREA_5);
  
  add128((uint128 *)srng_v,(uint128 *)srng_v, (uint128 *)add);
  sspAesEncryptHW_keylocation((uint8*)srng_v, (uint8*) srng_tmp2, KEY_AREA_5);
  
  for( i=0; i<RNG_BLOCK_LENGTH_BYTES; i++)
  {
    psrng_key[i] = psrng_tmp1[i] ^ entropy[i];
    psrng_v[i] = psrng_tmp2[i] ^ entropy[i+RNG_BLOCK_LENGTH_BYTES];
  }
  AESLoadKey( (uint8 *)srng_key, KEY_AREA_5);
}

/*******************************************************************************
 * @fn          srng_update
 *
 * @brief     Update SRNG  
 *
 * input parameters
 *
 * @param addData additional data
 *
 * output parameters
 *
 * None
 *
 * @return AES_SUCCESS if successful
 *******************************************************************************
 */
uint8 srng_update(uint8 * addData)
{
  uint32 srng_tmp1[RNG_BLOCK_LENGTH_32BITS];
  uint32 srng_tmp2[RNG_BLOCK_LENGTH_32BITS];
  uint8 *psrng_v = (uint8 *)srng_v;
  uint8 *psrng_key = (uint8 *)srng_key;
  uint8 *psrng_tmp1 = (uint8 *)srng_tmp1;
  uint8 *psrng_tmp2 = (uint8 *)srng_tmp2;
  uint8 i;
  
  reseed_interval++;
  add128((uint128 *)srng_v,(uint128 *)srng_v, (uint128 *)add);

  AESLoadKey( (unsigned char *)srng_key, KEY_AREA_5);
  sspAesEncryptHW_keylocation((uint8*)srng_v, (uint8*) srng_tmp1, KEY_AREA_5);
  
  add128((uint128 *)srng_v,(uint128 *)srng_v, (uint128 *)add);
  sspAesEncryptHW_keylocation((uint8*)srng_v, (uint8*) srng_tmp2, KEY_AREA_5);
  
  for( i=0; i<RNG_BLOCK_LENGTH_BYTES; i++)
  {
    psrng_key[i] = psrng_tmp1[i] ^ addData[i];
    psrng_v[i] = psrng_tmp2[i] ^ addData[i];
  }
  return (AESLoadKey( (uint8 *)srng_key, KEY_AREA_5));
}

/*******************************************************************************
 * @fn          srng_generate
 *
 * @brief     Generate SRNG output  
 *
 * input parameters
 *
 * @param Pointer to SRNG Output
 * @param Length in bytes
 * @param addData pointer to Additional Data. The data is 16 bytes in size
 *
 * output parameters
 *
 * @param output SRNG output 
 *
 * @return SRNG_SUCCESS if successful
 *******************************************************************************
 */
uint8 srng_generate(uint8 *out, uint16 len, uint8 *addData)
{
  /* len is uint16 because the max number of bits per request 
   * is 2^19 which is 2^16 in bytes 
   */
  uint32 srng_tmp_v[RNG_BLOCK_LENGTH_32BITS];
  uint8 *psrng_tmp_v = (uint8 *)srng_tmp_v;
  
  uint32 j =0;
  uint32 i, bytesneeded;
  uint8 adddata[RNG_BLOCK_LENGTH_BYTES];
  
  
  if(reseed_interval > reseed_counter)
  {
    return RNG_INIT_ERROR;
  }
  
  if(srng_v[0] >= SRNGV_CHECK_VAL)
  {
    if(srng_v[1] == MAX_32BIT_VAL && 
       srng_v[2] == MAX_32BIT_VAL && 
       srng_v[3] == MAX_32BIT_VAL)
    {
      return RNG_INIT_ERROR;
    }
  }
  
  if(addData == NULL)
  {
    for(i=0; i<RNG_BLOCK_LENGTH_BYTES; i++)
    {
      adddata[i] = 0;
    }
  }
  else
  {
    for(i=0; i<RNG_BLOCK_LENGTH_BYTES; i++)
    {
      adddata[i] = addData[i];
    } 
  }
  
  while(len)
  {
    bytesneeded = MIN(len, RNG_BLOCK_LENGTH_BYTES);
    add128((uint128 *)srng_v,(uint128 *)srng_v, (uint128 *)add);
    sspAesEncryptHW_keylocation((uint8 *)srng_v, psrng_tmp_v, KEY_AREA_5);
    
    for( i=0; i < bytesneeded; i++)
    {
      out[j] = psrng_tmp_v[i];
      j++;
    }
    len -= bytesneeded; 
  }
  srng_update((uint8 *)adddata);
  
  return SRNG_SUCCESS;
}