/*
 * Copyright (c) 2013, Nguyen Quang Huy
 * All rights reserved.
 *
 * This file is part of the Contiki operating system.
 *
 */

#include "hal_types.h"
#include "hal_mcu.h"
#include "usci_spi.h"
#include "fram.h"

volatile uint16_t loopDelay;
/*---------------------------------------------------------------------------------------*/
error_t fram_init(uint8_t spi_mode)
{
  CSSEL &= ~(1 << CSPIN);   /* I/O selected */
  CSDIR |= (1 << CSPIN);
  CS_DISABLE();

  VDDSEL &= ~(1 << VDDPIN);
  VDDDIR |= (1 << VDDPIN);
  VDD_ENABLE();

  WPSEL &= ~(1 << WPPIN);   /* I/O selected */
  WPDIR |= (1 << WPPIN);
  WP_DISABLE();

  usci_spi_init(spi_mode);

  /* Test SPI Operation */
  uint8_t pDeviceID[4];
  fram_readDeviceID(pDeviceID);
  if (pDeviceID[0] != 0x04 || pDeviceID[1] != 0x7F || pDeviceID[2] != 0x28 || pDeviceID[3] != 0x03)
  {
    return ERROR_INIT_FAIL;
  }

  return OK;
}

/*---------------------------------------------------------------------------------------*/
static void sendAddr(uint32_t addr){
  usci_spi_sendByte((addr & 0xFF0000) >> 16);
  usci_spi_sendByte((addr & 0xFF00) >> 8);
  usci_spi_sendByte(addr & 0xFF);
}

/*---------------------------------------------------------------------------------------*/
static void fram_writeEnable(void)
{
  CS_ENABLE();
  usci_spi_sendByte(CMD_WREN);
  CS_DISABLE();
}

/*---------------------------------------------------------------------------------------*/
static void fram_writeDisable(void)
{
  CS_ENABLE();
  usci_spi_sendByte(CMD_WRDI);
  CS_DISABLE();
}

/*---------------------------------------------------------------------------------------*/
error_t fram_readMemory(uint32_t startAddr, uint8_t *pData, uint16_t size)
{
  uint16_t i;

  CS_ENABLE();
  usci_spi_sendByte(CMD_READ);
  sendAddr(startAddr);
  for(i = 0; i < size; i++)
  {
    pData[i] = usci_spi_getByte();
  }
  CS_DISABLE();

  return OK;
}

/*---------------------------------------------------------------------------------------*/
error_t fram_fastReadMemory(uint32_t startAddr, uint8_t *pData, uint16_t size)
{
  CS_ENABLE();
  usci_spi_sendByte(CMD_FSTRD);
  CS_ENABLE();

  return OK;
}

/*---------------------------------------------------------------------------------------*/
error_t fram_writeByte(uint32_t startAddr, uint8_t data)
{
  fram_writeEnable();

  CS_ENABLE();
  usci_spi_sendByte(CMD_WRITE);
  sendAddr(startAddr);
  usci_spi_sendByte(data);
  CS_DISABLE();

  fram_writeDisable();

  return OK;
}

/*---------------------------------------------------------------------------------------*/
error_t fram_writeMemory(uint32_t startAddr, const uint8_t *pData, uint16_t size)
{
  uint16_t i;

  fram_writeEnable();

  CS_ENABLE();
  usci_spi_sendByte(CMD_WRITE);
  sendAddr(startAddr);
  for(i = 0; i < size; i++)
  {
    usci_spi_sendByte(pData[i]);
  }
  CS_DISABLE();

  fram_writeDisable();

  return OK;
}

/*---------------------------------------------------------------------------------------*/
error_t fram_readDeviceID(uint8_t* pDeviceID)
{
  unsigned char i;

  usci_spi_getByte();
  
  CS_ENABLE();
  for (uint16_t j=0; j<5000; j++){
    loopDelay++;
  }
  
  usci_spi_sendByte(CMD_RDID);
  for(i = 0; i < 4; i++)
  {
    pDeviceID[i] = usci_spi_getByte();
  }
  CS_DISABLE();

  return OK;
}

/*---------------------------------------------------------------------------------------*/
//error_t fram_sleepMode(void)
//{
//  CS_ENABLE();
//  usci_spi_sendByte(CMD_SLEEP);
//  CS_DISABLE();

//  return OK;
//}

/*---------------------------------------------------------------------------------------*/
//uint8_t fram_readStatusRegister(void)
//{
//  return (usci_spi_sendByte(CMD_RDSR));
//}

/*---------------------------------------------------------------------------------------*/
//void fram_writeStatusRegister(uint8_t val)
//{
//  fram_writeEnable();
//  usci_spi_sendByte(CMD_WRSR);
//  fram_writeDisable();
//}
