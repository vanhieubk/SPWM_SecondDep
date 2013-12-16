/*
 * Copyright (c) 2013, Nguyen Quang Huy
 * All rights reserved.
 *
 * This file is part of the Contiki operating system.
 *
 */
#ifndef __FRAM_H__
#define __FRAM_H__

#ifdef __cplusplus
extern "C" {
#endif


#include "usci_spi.h"


#define FRAM_MODE0  SPI_MODE0
#define FRAM_MODE3  SPI_MODE3

/* Define I/O for controlling CS, WP and HOLD pin */
#define CSSEL       P1SEL
#define CSDIR       P1DIR
#define CSOUT       P1OUT
#define CSPIN       0       /* P1.0 */

#define VDDSEL      P7SEL
#define VDDDIR      P7DIR
#define VDDOUT      P7OUT
#define VDDPIN      7       /* P7.7 */

#define WPSEL       P1SEL
#define WPDIR       P1DIR
#define WPOUT       P1OUT
#define WPPIN       1       /* P1.1 */

#define CS_DISABLE()       (CSOUT |= (1 << CSPIN))
#define CS_ENABLE()        (CSOUT &= ~(1 << CSPIN))

#define VDD_ENABLE()         (VDDOUT &= ~(1 << VDDPIN))
#define VDD_DISABLE()        (VDDOUT |= (1 << VDDPIN))

#define WP_DISABLE()       (WPOUT |= (1 << WPPIN))
#define WP_ENABLE()        (WPOUT &= ~(1 << WPPIN))


/* FRAM Command */
#define CMD_WREN        0x06  /**< Set Write Enable Latch     0000 0110B */
#define CMD_WRDI        0x04  /**< Reset Write Enable Latch   0000 0100B */
#define CMD_RDSR        0x05  /**< Read Status Register       0000 0101B */
#define CMD_WRSR        0x01  /**< Write Status Register      0000 0001B */
#define CMD_READ        0x03  /**< Read Memory Code           0000 0011B */
#define CMD_WRITE       0x02  /**< Write Memory Code          0000 0010B */
#define CMD_RDID        0x9F  /**< Read Device ID             1001 1111B */
#define CMD_FSTRD       0x0B  /**< Fast Read Memory Code      0000 1011B */
#define CMD_SLEEP       0xB9  /**< Sleep Mode                 1011 1001B */

/* Error code */
typedef enum {
  OK               = (0),
  ERROR_INIT_FAIL  = (1),
  ERROR_WRITE_FAIL = (2)
} error_t;

/* FRAM Interface */
error_t fram_init(uint8_t spi_mode);

error_t fram_readMemory(uint32_t startAddr, uint8_t *pData, uint16_t size);
error_t fram_fastReadMemory(uint32_t startAddr, uint8_t *pData, uint16_t size);
error_t fram_writeMemory(uint32_t startAddr, const uint8_t *pData, uint16_t size);
error_t fram_writeByte(uint32_t startAddr, uint8_t data);

error_t fram_readDeviceID(uint8_t *pDeviceID);
//error_t fram_sleepMode(void);
//uint8_t fram_readStatusRegister(void);

#endif /* __FRAM_H__ */

