#ifndef __SIM900_H
#define __SIM900_H

#include "hal_defs.h"

#define SIM_NRST_PORT		  P3OUT
#define SIM_NRST_DDR			P3DIR
#define SIM_NRST_PIN			BV(6)

#define SIM_TXD_PORT		  P5OUT
#define SIM_TXD_DDR			  P5DIR
#define SIM_TXD_PIN			  BV(7)

#define SIM_RXD_PORT		  P5OUT
#define SIM_RXD_DDR			  P5DIR
#define SIM_RXD_PIN			  BV(6)

#define SIM_RI_PORT		    P2OUT
#define SIM_RI_DDR			  P2DIR
#define SIM_RI_PIN			  BV(4)

#define SIM_NPWR_PORT		  P8OUT
#define SIM_NPWR_DDR			P8DIR
#define SIM_NPWR_PIN			BV(3)

#define SIM_STATUS_PORT		P8OUT
#define SIM_STATUS_DDR		P8DIR
#define SIM_STATUS_PIN		BV(2)

#define SIM_DCD_PORT		  P7OUT
#define SIM_DCD_DDR			  P7DIR
#define SIM_DCD_PIN			  BV(2)

#define SIM_DTR_PORT		  P7OUT
#define SIM_DTR_DDR			  P7DIR
#define SIM_DTR_PIN			  BV(3)


/* primitive output changing MACROs (do not depend on your board)*/
#define SIM_PIN_MODE_OUT(port, pin) (port) |=  (pin)
#define SIM_PIN_MODE_IN(port, pin)  (port) &= ~(pin)
#define SIM_SET_PIN(port, pin)  	  (port) |=  (pin)
#define SIM_CLR_PIN(port, pin) 	    (port) &= ~(pin)

/************************************************/
void SIM_Init(void);
void SIM_InitPin(void);
void SIM_ResetActive(void);
void SIM_ResetDeactive(void);
void SIM_PowerOff(void);
void SIM_PowerOn(void);

#endif /* __SIM900_H */
