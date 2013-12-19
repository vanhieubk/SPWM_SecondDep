#include "packet.h"
#include "hal_uart.h"

static void _PrintAlivePkt(alivePara_t* alivePara);
static void _PrintSensingPkt(sensingPara_t* sensingPara);


/***************************************************/
static void _PrintAlivePkt(alivePara_t* alivePara){
  HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "ALIVE: node(", alivePara->nodeId, 10);
  HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, ")  time: ", alivePara->curTime, 10);
}


static void _PrintSensingPkt(sensingPara_t* sensingPara){
  HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "SENSING: node(", sensingPara->nodeId, 10);
  HalUARTPrintStrAndUInt(HAL_UART_PORT_0,") time: ", sensingPara->time, 10);
  HalUARTPrintStrAndUInt(HAL_UART_PORT_0,"  run: ", sensingPara->run, 10);
  HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0,"  store: ", sensingPara->storeBlock, 10);
  SS_Print(&(sensingPara->sensingData));
}



/******************************************************/
void PKT_Print(pkt_t* pkt){
  switch(pkt->pktType){
    case PKT_ALIVE_TYPE:
      _PrintAlivePkt(&(pkt->pktPara.alivePara));
      break;

    case PKT_SENSING_TYPE:
      _PrintSensingPkt(&(pkt->pktPara.sensingPara));
    break;
  }
}


