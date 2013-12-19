#ifndef __SENSING_H
#define __SENSING_H

#include "arch_port.h"

typedef struct {
  uint16_t pHAdc[16];

  uint16_t vddAdcAvg;
  uint16_t intTmpAdcAvg;
  uint16_t pHAdcAvg;
  uint16_t extTmpAdcAvg;

  uint16_t vddVal;
  uint16_t intTmpVal;
  uint16_t pHVal;
  uint16_t extTmpRes;
  uint16_t extTmpVal;
} sensing_t;

void SS_Init(void);
void SS_Prepare(void);
void SS_Measure(sensing_t* ssResult);
void SS_Shutdown(void);
void SS_Print(sensing_t* ssResult);

#endif
