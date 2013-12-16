/*
 * Copyright (c) 2013, Nguyen Quang Huy
 * All rights reserved.
 *
 * This file is part of the Contiki operating system.
 *
 */
#ifndef _SPI_CONFIG_H_
#define _SPI_CONFIG_H_

/* Select which USCI module to use */
#define SPI_USE_USCI    3
/**<      0 = USCIA0 \n
          1 = USCIA1 \n
          2 = USCIB0 \n
          3 = USCIB1
**/

/* Select which clock source to use */
#define SPI_CLK_SRC     2
/**<      1 = ACLK   \n
          2 = SMCLK
**/

/* SPI Clock division. Must be 4 or greater */
#define SPI_CLK_DIV     25

/* Byte that is transmitted during read operations */
#define DUMMY_CHAR     (0xFF)


#endif /* _SPI_CONFIG_H_ */
