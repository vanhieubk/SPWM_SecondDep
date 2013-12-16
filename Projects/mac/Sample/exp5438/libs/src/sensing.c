#include "hal_types.h"
#include "hal_adc.h"
#include "hal_mcu.h"
#include "hal_uart.h"

#include "sensing.h"

#define ADC_PORT_DIR     P6DIR
#define ADC_PORT_REN     P6REN
#define ADC_PORT_SEL     P6SEL

#define ADC_PH_CHANNEL        ADC12INCH_6
#define ADC_EXT_TMP_CHANNEL   ADC12INCH_4
#define ADC_INT_TMP_CHANNEL   ADC12INCH_10
#define ADC_VDD_CHANNEL       ADC12INCH_11
//////////////////////////////////////////////
uint16_t AvgWithFilter(uint16_t inData[16]);
uint16_t _SS_GetAdc(uint8_t channel);

int32_t  intTmpOffsetCalib = -30; /* mV */
int32_t  extTmpOffsetCalib = 0; /* mV */
int32_t  pHOffsetCalib     = 0; /* mV */

//////////////////////////////////////////////
/**
  * @brief Set pin mode
  */
void SS_Init(void){
  ADC_PORT_DIR = 0; //input
  ADC_PORT_REN = 0; //no pull-up/pull-down resistor
  ADC_PORT_SEL = 0xFF; //peripheral I/O

  ADC12CTL0 = 0; /* off ADC to config */
  /* disable all interrupt */
  ADC12IE = 0;

  REFCTL0 &= ~REFMSTR; /*allo ADC reg config REF */
}



/**
  * @brief Turn on REF, TEMP, GPIO
  */
void SS_Prepare(void){
  //turn on REF, TEMP, GPIO, ADC
  ADC12CTL0 = 0; /* off ADC to config */
  /* 1024 clock, manual trigger, on ref 2.5V, on ADC */
  ADC12CTL0 = ADC12SHT1_12 + ADC12SHT0_12 + ADC12REF2_5V + ADC12REFON + ADC12ON;
  /* preDivide 1, on Temp, Resolution 12, unsigned binary, 5max 200ksps, out ref continously */
  ADC12CTL2 = ADC12RES_2  + ADC12REFOUT;
  /* StartMem0, SH trigger, SAMCON timer, clk div 8, Master clock, one channel */
  ADC12CTL1 = ADC12CSTARTADD_0 + ADC12SHS_0 + ADC12SHP + ADC12DIV_7 + ADC12SSEL_2 + ADC12CONSEQ_0;
}



/**
  * @brief Set pin mode, set ADC clock
  */
void SS_Measure(sensing_t *ssResult){
  uint16_t index;
  uint16_t vddAdc[16], intTmpAdc[16];
  int32_t val, tmp;
  for (index=0; index<16; index++){
    vddAdc[index]              = _SS_GetAdc(ADC_VDD_CHANNEL);
    intTmpAdc[index]           = _SS_GetAdc(ADC_INT_TMP_CHANNEL);
    ssResult->pHAdc[index]     = _SS_GetAdc(ADC_PH_CHANNEL);
    ssResult->extTmpAdc[index] = _SS_GetAdc(ADC_EXT_TMP_CHANNEL);
  }

  /* calculate Average value */
  ssResult->vddAdcAvg = AvgWithFilter(vddAdc);
  ssResult->intTmpAdcAvg = AvgWithFilter(intTmpAdc);
  ssResult->pHAdcAvg = AvgWithFilter(ssResult->pHAdc);
  ssResult->extTmpAdcAvg = AvgWithFilter(ssResult->extTmpAdc);

  /* calculate measured data */
  /* Vdd = (2.5/2^12)*N*2 (vol) = (5000*N)/4096 (miliVol) */
  val = ((int32_t) ssResult->vddAdcAvg)* ((int32_t)5000);
  ssResult->vddVal = (uint16_t) (val / ((int32_t)4096));

  /*  V= -offset + 680+2.25*t (mV) <=> (2500/2^12)*N = -offset + 680+2.25*t
   => t = [(2500/4096)*N - 680 + offset]/2.25
   => t = [(2500*N - (680-offset)*4096)/4096)]/2.25 = (2500*N - 680*4096)/9216 (degree) */
  val = ((int32_t) ssResult->intTmpAdcAvg)* ((int32_t)2500);
  val = val - ((int32_t) (((int32_t) 680) - intTmpOffsetCalib)*((int32_t) 4096));
  ssResult->intTmpVal = (uint16_t) (val / ((int32_t)9216));

  /* V = Vref.R/(R+Rref)  <=> V.R + V.Rref = Vref.R
  => R = V.Rref/(Vref-V) */
  val = (((int32_t) ssResult->extTmpAdcAvg)* ((int32_t)2500)) / ((int32_t) 4096);
  tmp = val * ((int32_t) 47000);
  val = tmp / (((int32_t) 2500) - val);
  ssResult->extTmpRes = val;
  if (val < 17700)       {ssResult->extTmpVal = 36;}
  else if (val < 18200)  {ssResult->extTmpVal = 35;}
  else if (val < 18800)  {ssResult->extTmpVal = 34;}
  else if (val < 19350)  {ssResult->extTmpVal = 33;}
  else if (val < 19900)  {ssResult->extTmpVal = 32;}
  else if (val < 20450)  {ssResult->extTmpVal = 31;}
  else if (val < 21000)  {ssResult->extTmpVal = 30;}
  else if (val < 22200)  {ssResult->extTmpVal = 29;}
  else if (val < 23500)  {ssResult->extTmpVal = 28;}
  else if (val < 25000)  {ssResult->extTmpVal = 27;}
  else                   {ssResult->extTmpVal = 26;}
  
  /* V = Vref/2 + PH_OFFSET + (7-pH)*PH_SLOPE (mV)
       = Vref/2 + (PH_OFFSET_ZERO + t*PH_OFFSET_DRIFT) + (7-pH)*(59.2 + t*0.2)
 <=> 2500N/4096 - 2500/2 ~= 0 + (7-pH)*(59.2 + t*0.2)
  => pH = 7-(2500N-5120000)/[4096*(59.2 + t*0.2)]
  => pH = 350-[(2500N-5120000)*250)/[4096*(296 + t)]  (0.02pH) */
  val = ((int32_t) ssResult-> pHAdcAvg)* ((int32_t)2500);
  val = (val - ((int32_t) 5120000)) * ((int32_t)250);
  val = val / ( ((int32_t)4096)*(((int32_t)310) + ((int32_t)ssResult->extTmpVal)) );
  ssResult->pHVal = (uint16_t) (2*(250 - val));
  

}



void SS_Shutdown(void){
  //turn off REF, TEMP, GPIO, ADC
  ADC12CTL0 &= ~ADC12ENC; // Disable ADC
  ADC12CTL0 = 0;          // Turn off reference (must be done AFTER clearing ENC).
  ADC12CTL2 = ADC12TCOFF; // Turn off RefOUT, temperature sensor
}




uint16_t AvgWithFilter(uint16_t inData[16]){
  uint16_t sum, avg;
  uint16_t delta[16], order[16];
  uint16_t index, pos, swap;

  sum = 0;
  for (index=0; index<16; index++){
    sum = sum + inData[index];
    order[index] = index;
  }
  avg = sum >> 4;
  /* calculate delta */
  for (index=0; index<16; index++){
    if (inData[index] > avg){
      delta[index] = inData[index] - avg;
    }
    else{
      delta[index] = avg - inData[index];
    }
  }

  /* find 8 smallest delta */
  for (index=0; index<16; index++){
    for (pos=index+1; pos<16; pos++){
      if (delta[pos] < delta[index]){
        swap = delta[index];
        delta[index] = delta[pos];
        delta[pos]=swap;
        swap = order[index];
        order[index] = order[pos];
        order[pos]=swap;
      }
    }
  }

  /*recalculate sum*/
  sum = 0;
  for (index=0;index<8; index++){
    sum = sum + inData[(order[index])];
  }
  return (sum >> 3);
}



uint16_t _SS_GetAdc(uint8_t channel){
  ADC12CTL0  &= ~ (ADC12ENC + ADC12SC); /*disable ADC to config */
  ADC12MCTL0  = ADC12EOS + ADC12SREF1 + channel;
  ADC12CTL0  |= (ADC12ENC + ADC12SC);
  while (ADC12IFG == 0){
    /*wait, do nothing*/
  };
  ADC12IFG = 0;
  return ADC12MEM0;
}


void SS_Print(sensing_t* ssResult){
  HalUARTWriteString(HAL_UART_PORT_0,"\n=============================================================\n");
  HalUARTWriteString(HAL_UART_PORT_0,"nodeId: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->nodeId, 10);
  HalUARTWriteString(HAL_UART_PORT_0," runTime: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->runTime, 10);
  HalUARTWriteString(HAL_UART_PORT_0," blk: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->storeBlock, 10);
  HalUARTWriteString(HAL_UART_PORT_0," sysClk: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->sendSysClk, 10);
  HalUARTWriteStringValue(HAL_UART_PORT_0," sqc: ", ssResult->sequence, 10);
  
  HalUARTWriteString(HAL_UART_PORT_0,"----------\npHAdc: ");
  for (int i=0; i<16; i++){
    HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->pHAdc[i], 10);
    HalUARTWriteString(HAL_UART_PORT_0," ");
  }
  HalUARTWriteString(HAL_UART_PORT_0,"\nextTmpAdc : ");
  for (int i=0; i<16; i++){
    HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->extTmpAdc[i], 10);
    HalUARTWriteString(HAL_UART_PORT_0," ");
  }

  HalUARTWriteString(HAL_UART_PORT_0, "vddAvg: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->vddAdcAvg, 10);
  HalUARTWriteString(HAL_UART_PORT_0, " iTmpAvg: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->intTmpAdcAvg, 10);
  HalUARTWriteString(HAL_UART_PORT_0, " pHAvg: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->pHAdcAvg, 10);
  HalUARTWriteStringValue(HAL_UART_PORT_0, " eTmpAvg: ", ssResult->extTmpAdcAvg, 10);

  HalUARTWriteString(HAL_UART_PORT_0, "vddVal: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->vddVal, 10);
  HalUARTWriteString(HAL_UART_PORT_0, " iTmpVal: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->intTmpVal, 10);
  HalUARTWriteString(HAL_UART_PORT_0, " pHVal: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->pHVal, 10);
  HalUARTWriteString(HAL_UART_PORT_0, " eTmpRes: ");
  HalUARTWriteNumber(HAL_UART_PORT_0, ssResult->extTmpRes, 10);
  HalUARTWriteStringValue(HAL_UART_PORT_0, " eTmpVal: ", ssResult->extTmpVal, 10);
}
