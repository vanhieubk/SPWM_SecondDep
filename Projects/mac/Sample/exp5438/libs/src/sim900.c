#include "hal_types.h"
#include "hal_mcu.h"
#include "sim900.h"




void SIM_Init(){
  uint32 i;

  SIM_PowerOn();
  SIM_ResetActive();
  SIM_InitPin();
  for (i=0; i<100000; i++);
  /*delay */
  SIM_ResetDeactive();
  /* delay */
  for (i=0; i<10000000; i++);
}


void SIM_InitPin(){
  SIM_PIN_MODE_OUT(SIM_NRST_DDR, SIM_NRST_PIN);
  SIM_PIN_MODE_OUT(SIM_NPWR_DDR, SIM_NPWR_PIN);
  SIM_PIN_MODE_OUT(SIM_TXD_DDR,  SIM_TXD_PIN);

  SIM_PIN_MODE_IN(SIM_RI_DDR,  SIM_RI_PIN);
  SIM_PIN_MODE_IN(SIM_RXD_DDR, SIM_RXD_PIN);
}


void SIM_ResetActive(){
  SIM_CLR_PIN(SIM_NRST_PORT, SIM_NRST_PIN);
}


void SIM_ResetDeactive(){
  SIM_SET_PIN(SIM_NRST_PORT, SIM_NRST_PIN);
}


void SIM_PowerOff(){
  SIM_SET_PIN(SIM_NPWR_PORT, SIM_NPWR_PIN);
}


void SIM_PowerOn(){
  SIM_CLR_PIN(SIM_NPWR_PORT, SIM_NPWR_PIN);
}