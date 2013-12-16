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
#include "spi_internal.h"

void usci_spi_init(uint8_t spi_mode)
{
  /* Set UCSWRST - Hold peripheral (USCI) in reset state               */
  SPI_UCCTL1 |= UCSWRST;

  /* MSB first, 8-bit data, Master mode, 3-pin SPI, Synchronous mode   */
  SPI_UCCTL0 = spi_mode + UCMSB + UCMST + UCSYNC;
  SPI_UCCTL1 = (SPI_CLK_SRC << 6) + UCSWRST;
  SPI_UCBR = SPI_CLK_DIV;

  /* Configure port */
  #if (SPI_USE_USCI == 1)
    P3SEL |= 0x40;    /* P3.6    USCI_A1 option selected   0100 0000B */
    P5SEL |= 0x0C0;   /* P5.7,6  USCI_A1 option selected   1100 0000  */

    P5DIR &= ~0x80;   /* P5.7 = USCI_A1 MISO as input                 */
    P5DIR |= 0x40;    /* P5.6 = USCI_A1 MOSI as output                */
    P3DIR |= 0x40;    /* P3.6 = USCI_A1 SCK as output                 */

    P3OUT &= ~0x40;   /* SCK = 0                                      */

  #elif (SPI_USE_USCI == 3)
    P3SEL |= 0x80;    /* P3.7    USCI_B1 option selected   1000 0000  */
    P5SEL |= 0x30;    /* P5.5,4  USCI_B1 option selected   0011 0000  */

    P5DIR &= ~0x10;   /* P5.4 = USCI_B1 MISO as input                 */
    P3DIR |= 0x80;    /* P3.7 = USCI_B1 MOSI as output                */
    P5DIR |= 0x20;    /* P5.5 = USCI_B1 SCK as output                 */

    P5OUT &= 0x20;    /* SCK = 0                                      */
  #endif

  #if(SPI_USE_USCI < 2)
    SPI_UCMCTL = 0;
  #endif

  // Clear UCSWRST - USCI reset released for operation
  SPI_UCCTL1 &= ~UCSWRST;
}

//--------------------------------------------------------------------------------------------------
uint8_t usci_spi_sendByte(uint8_t data)
{
  volatile uint8_t x;
  SPI_UCTXBUF = data;                     /* write                           */
  while ((SPI_UCIFG & UCRXIFG) == 0);     /* wait for transfer to complete   */
  x = SPI_UCRXBUF;                        /* dummy read to clear the RX flag */
  return x;
}
