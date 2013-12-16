/*
 * Copyright (c) 2013, Nguyen Quang Huy
 * All rights reserved.
 *
 * This file is part of the Contiki operating system.
 *
 */
#ifndef __USCI_SPI_H__
#define __USCI_SPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "arch_port.h"
#include "spi_config.h"

#define SPI_MODE0    (0+UCCKPH)          /**< brief CPOL = 0, CPHA = 0     */
#define SPI_MODE1    (0+UCCKPL+UCCKPH)   /**< brief CPOL = 0, CPHA = 1     */
#define SPI_MODE2    (0)                 /**< brief CPOL = 1, CPHA = 0     */
#define SPI_MODE3    (0+UCCKPL)          /**< brief CPOL = 1, CPHA = 1     */

void usci_spi_init(uint8_t spi_mode);
uint8_t usci_spi_sendByte(uint8_t data); /**< send & get byte concurrently */

#define usci_spi_getByte()    usci_spi_sendByte(DUMMY_CHAR)

#endif /* __USCI_SPI_H__ */

