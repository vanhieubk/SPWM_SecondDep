/*
 * Copyright (c) 2013, Nguyen Quang Huy
 * All rights reserved.
 *
 * This file is part of the Contiki operating system.
 *
 */
#ifndef _SPI_INTERNAL_H_
#define _SPI_INTERNAL_H_


#include "spi_config.h"
//==================================================================================================

#if SPI_CLK_SRC > 2
    #error "Invalid SPI_CLK_SRC in spi_config.h"
#elif SPI_CLK_SRC < 1
    #error "Invalid SPI_CLK_SRC in spi_config.h"
#endif


#if SPI_USE_USCI == 0
    #if defined(__MSP430_HAS_USCI_A0__)
        #define SPI_UCCTL0   UCA0CTL0
        #define SPI_UCCTL1   UCA0CTL1
        #define SPI_UCBR     UCA0BRW
        #define SPI_UCMCTL   UCA0MCTL
        #define SPI_UCSTAT   UCA0STAT
        #define SPI_UCRXBUF  UCA0RXBUF
        #define SPI_UCTXBUF  UCA0TXBUF
        #define SPI_UCIE     UCA0IE
        #define SPI_UCIFG    UCA0IFG
        #define SPI_UCIV     UCA0IV
    #else
        #error "Invalid SPI_USE_USCI in spi_config.h"
    #endif
#elif SPI_USE_USCI == 1
    #if defined(__MSP430_HAS_USCI_A1__)
        #define SPI_UCCTL0   UCA1CTL0
        #define SPI_UCCTL1   UCA1CTL1
        #define SPI_UCBR     UCA1BRW
        #define SPI_UCMCTL   UCA1MCTL
        #define SPI_UCSTAT   UCA1STAT
        #define SPI_UCRXBUF  UCA1RXBUF
        #define SPI_UCTXBUF  UCA1TXBUF
        #define SPI_UCIE     UCA1IE
        #define SPI_UCIFG    UCA1IFG
        #define SPI_UCIV     UCA1IV
    #else
        #error "Invalid SPI_USE_USCI in spi_config.h"
    #endif
#elif SPI_USE_USCI == 2
    #if defined(__MSP430_HAS_USCI_B0__)
        #define SPI_UCCTL0   UCB0CTL0
        #define SPI_UCCTL1   UCB0CTL1
        #define SPI_UCBR     UCB0BRW
        #define SPI_UCMCTL   UCB0MCTL
        #define SPI_UCSTAT   UCB0STAT
        #define SPI_UCRXBUF  UCB0RXBUF
        #define SPI_UCTXBUF  UCB0TXBUF
        #define SPI_UCIE     UCB0IE
        #define SPI_UCIFG    UCB0IFG
        #define SPI_UCIV     UCB0IV
    #else
        #error "Invalid SPI_USE_USCI in spi_config.h"
    #endif
#elif SPI_USE_USCI == 3
        #define SPI_UCCTL0   UCB1CTL0
        #define SPI_UCCTL1   UCB1CTL1
        #define SPI_UCBR     UCB1BRW
        #define SPI_UCMCTL   UCB1MCTL
        #define SPI_UCSTAT   UCB1STAT
        #define SPI_UCRXBUF  UCB1RXBUF
        #define SPI_UCTXBUF  UCB1TXBUF
        #define SPI_UCIE     UCB1IE
        #define SPI_UCIFG    UCB1IFG
        #define SPI_UCIV     UCB1IV
#else
    #error "Invalid SPI_USE_USCI in spi_config.h"
#endif

#endif  /* _SPI_INTERNAL_H_ */

