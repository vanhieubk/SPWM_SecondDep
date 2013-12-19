/* Hal Driver includes */
#include "hal_types.h"
#include "hal_key.h"
//#include "hal_timer.h"
//#include "hal_drivers.h"
#include "hal_led.h"
#include "hal_uart.h"
/* OS includes */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OSAL_PwrMgr.h"
/* Application Includes */
#include "OnBoard.h"
/* For Random numbers */
#include "mac_radio_defs.h"
/* MAC Application Interface */
#include "mac_api.h"
#include "mac_main.h"
/* Application */
#include "nwk_comm.h"
#include "gateway.h"
#include "fram.h"
#include "packet.h"
#include "sensing.h"
#include "sim900.h"

/**** DEFINE   ****/
#define UART0_RX_BUF_SIZE        128
#define UART0_TX_BUF_SIZE        1028

#define GW_HEADER_LENGTH         4             /* Header includes DataLength + DeviceShortAddr + Sequence */
#define GW_ECHO_LENGTH           8             /* Echo packet */
#define GW_MAX_DEVICE_NUM        32            /* Maximun number of devices can associate with the coordinator */


/* Size table for MAC structures */
const CODE uint8 gw_cbackSizeTable [] =
{
  0,                                   /* unused */
  sizeof(macMlmeAssociateInd_t),       /* MAC_MLME_ASSOCIATE_IND */
  sizeof(macMlmeAssociateCnf_t),       /* MAC_MLME_ASSOCIATE_CNF */
  sizeof(macMlmeDisassociateInd_t),    /* MAC_MLME_DISASSOCIATE_IND */
  sizeof(macMlmeDisassociateCnf_t),    /* MAC_MLME_DISASSOCIATE_CNF */
  sizeof(macMlmeBeaconNotifyInd_t),    /* MAC_MLME_BEACON_NOTIFY_IND */
  sizeof(macMlmeOrphanInd_t),          /* MAC_MLME_ORPHAN_IND */
  sizeof(macMlmeScanCnf_t),            /* MAC_MLME_SCAN_CNF */
  sizeof(macMlmeStartCnf_t),           /* MAC_MLME_START_CNF */
  sizeof(macMlmeSyncLossInd_t),        /* MAC_MLME_SYNC_LOSS_IND */
  sizeof(macMlmePollCnf_t),            /* MAC_MLME_POLL_CNF */
  sizeof(macMlmeCommStatusInd_t),      /* MAC_MLME_COMM_STATUS_IND */
  sizeof(macMcpsDataCnf_t),            /* MAC_MCPS_DATA_CNF */
  sizeof(macMcpsDataInd_t),            /* MAC_MCPS_DATA_IND */
  sizeof(macMcpsPurgeCnf_t),           /* MAC_MCPS_PURGE_CNF */
  sizeof(macEventHdr_t),               /* MAC_PWR_ON_CNF */
  sizeof(macMlmePollInd_t)             /* MAC_MLME_POLL_IND */
};


/*****  LOCAL VARIABLEs  ********************************/
/* Beacon payload, this is used to determine if the device is not zigbee device */
uint8         gw_BeaconPayload[] = {0x22, 0x33, 0x44};
uint8         gw_BeaconPayloadLen = 3;
uint8         gw_SuperFrameOrder = NWK_MAC_SUPERFRAME_ORDER;
uint8         gw_BeaconOrder     = NWK_MAC_BEACON_ORDER;
/* Task ID */
uint8 GW_TaskId;
/* Coordinator and Device information */
sAddrExt_t    gw_ExtAddr         = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
uint16        gw_PanId           = NWK_PAN_ID;
uint16        gw_CoordShortAddr  = NWK_GW_SHORT_ADDR;
int8          gw_txPower = 19;
/* List of short addresses that the coordinator will use*/
uint16        gw_DevShortAddrList[] = { 0x0001, 0x0002, 0x0003, 0x0004, 0x0005,
                                        0x0006, 0x0007, 0x0008, 0x0009, 0x000A,
                                        0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
                                        0x0010, 0x0011, 0x0012, 0x0013, 0x0014,
                                        0x0015, 0x0016, 0x0017, 0x0018, 0x0019,
                                        0x001A, 0x001B, 0x001C, 0x001D, 0x001E,
                                        0x001F, 0x0020};
uint8         gw_NumOfDevices = 0; /* Current number of devices associated to the coordinator */

/* TRUE and FALSE value */
bool          gw_MACTrue  = TRUE;
bool          gw_MACFalse = FALSE;

/* flags used in the application */
bool          gw_IsStarted      = FALSE;
/* Structure that used for association response */
macMlmeAssociateRsp_t    gw_AssocRsp;

uint16  nodeShortAddrs[2];


uint16  stickDuration  = GW_DEFAULT_STICK_DURATION;
uint32  curStickTime   = 0;

pkt_t alivePacket;
/**** LOCAL FUNCTIONs DECLARATION ****/
void UART0Start(void);
void GW_UARTCallBack (uint8 port, uint8 event);
/* Setup routines */
void GW_CoordinatorStartup(uint8 usedChannel);
/* MAC related routines */
void ProcessInitPeriEvent(void);
void ProcessStickTimerEvent(void);
void ProcessingScanConfirm(macCbackEvent_t * pData);
void ProcessAssocIndEvent(macCbackEvent_t* pMsg);
void ProcessReceivingPacket(macMcpsDataInd_t* pData);
void GW_SendDataRequest(pktType_t pktType, uint8* pktPara, uint8 paraLen, uint16 dstShortAddr);



/*********  INIT FUNCTIONs  ********************/
/******************************************************/
void GW_Init(uint8 taskId){
  GW_TaskId = taskId;
  MAC_InitCoord();
  MAC_MlmeResetReq(TRUE);
  gw_BeaconOrder     = NWK_MAC_BEACON_ORDER;
  gw_SuperFrameOrder = NWK_MAC_SUPERFRAME_ORDER;

  HalLedSet(HAL_LED_2, HAL_LED_MODE_ON);
  UART0Start();   /* init UART0 for PC communication */
  osal_start_timerEx(GW_TaskId, GW_PREP_INIT_EVENT, 10000); /* delay 10s after power up */
}



void GW_CoordinatorStartup(uint8 usedChannel)
{
  macMlmeStartReq_t   startReq;

  /* Setup MAC_EXTENDED_ADDRESS */
  MAC_MlmeSetReq(MAC_EXTENDED_ADDRESS, &gw_ExtAddr);
  /* Setup MAC_SHORT_ADDRESS */
  MAC_MlmeSetReq(MAC_SHORT_ADDRESS, &gw_CoordShortAddr);
  /* Setup MAC_BEACON_PAYLOAD_LENGTH */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD_LENGTH, &gw_BeaconPayloadLen);
  /* Setup MAC_BEACON_PAYLOAD */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD, &gw_BeaconPayload);
  /* Enable RX */
  MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &gw_MACTrue);
  /* Set TXPower */
  MAC_MlmeSetReq(MAC_PHY_TRANSMIT_POWER_SIGNED, &gw_txPower);
  /* Setup MAC_ASSOCIATION_PERMIT */
  MAC_MlmeSetReq(MAC_ASSOCIATION_PERMIT, &gw_MACTrue);
  /* Fill in the information for the start request structure */
  startReq.startTime                = 0;
  startReq.panId                    = gw_PanId;
  startReq.logicalChannel           = usedChannel;
  startReq.beaconOrder              = gw_BeaconOrder;
  startReq.superframeOrder          = gw_SuperFrameOrder;
  startReq.panCoordinator           = TRUE;
  startReq.batteryLifeExt           = FALSE;
  startReq.coordRealignment         = FALSE;
  startReq.realignSec.securityLevel = FALSE;
  startReq.beaconSec.securityLevel  = FALSE;
  /* Call start request to start the device as a coordinator */
  MAC_MlmeStartReq(&startReq);
}



/*************  EVENT PROCESSING  ****************/
/************************************************************/
uint16 GW_ProcessEvent(uint8 taskId, uint16 events)
{
  uint8* pMsg;
  macCbackEvent_t* pData;

  if (events & SYS_EVENT_MSG){
    while ((pMsg = osal_msg_receive(GW_TaskId)) != NULL){
      switch ( *pMsg ){
        case MAC_MLME_SCAN_CNF:   /* NETWORK SCAN COMPLETED */
          ProcessingScanConfirm((macCbackEvent_t *) pMsg);
          break;

        case MAC_MLME_START_CNF:  /* COORDINATOR STARTED COMPLETED */
          pData = (macCbackEvent_t *) pMsg;
          if (pData->startCnf.hdr.status == MAC_SUCCESS){
            gw_IsStarted       = TRUE;
            HalUARTPrintStr(HAL_UART_PORT_0, "COOR Started\n");
          }
          else{
            osal_start_timerEx(GW_TaskId, GW_PREP_INIT_EVENT, 10000); /* delay 10s then rescan */
          }
          break;

        case MAC_MLME_ASSOCIATE_IND: /* HAS one DEVICE want ASSOCIATE */
          ProcessAssocIndEvent((macCbackEvent_t*) pMsg);
        break;

        case MAC_MCPS_DATA_IND: /* receiving packet */
          ProcessReceivingPacket((macMcpsDataInd_t*) pMsg);
        break;

        case MAC_MCPS_DATA_CNF:  /* MAC send data confirm */
          pData = (macCbackEvent_t *) pMsg;
          HalUARTPrintStr(HAL_UART_PORT_0, "Data response\n");

          mac_msg_deallocate((uint8**)&pData->dataCnf.pDataReq);
          break;

      } /* end switch */
      mac_msg_deallocate((uint8 **)&pMsg);
    } /* END while */
    return events ^ SYS_EVENT_MSG;
  }

  if (events & GW_PREP_INIT_EVENT){
    ProcessInitPeriEvent();
    return events ^ GW_PREP_INIT_EVENT;
  }

  if (events & GW_STICK_TIMER_EVENT){
    ProcessStickTimerEvent();
    return events ^ GW_STICK_TIMER_EVENT;
  }
  return 0;
}


/******  INIT peripheral event  **************/
void ProcessInitPeriEvent(){
  HalUARTPrintStr(HAL_UART_PORT_0, "\n\n*********************\n");
  HalUARTPrintStr(HAL_UART_PORT_0, "PROGRAM: start\n");

  if (ERROR_INIT_FAIL == fram_init(FRAM_MODE0)){ /* init FRAM */
    HalUARTPrintStr(HAL_UART_PORT_0, "FRAM: fail\n");
  }
  else{
    HalUARTPrintStr(HAL_UART_PORT_0, "FRAM: ok\n");
  }

  /* Start scan */
  HalUARTPrintStr(HAL_UART_PORT_0, "SCAN: start\n");
  NWK_ScanReq(MAC_SCAN_ED, GW_SCAN_DURATION);

  stickDuration  = GW_DEFAULT_STICK_DURATION;
  curStickTime   = 0;
  /* Start stick timer */
  if (SUCCESS == osal_start_reload_timer(GW_TaskId, GW_STICK_TIMER_EVENT, stickDuration)){
    HalUARTPrintStr(HAL_UART_PORT_0, "SYS TIMER: start\n");
  }
  else{
     HalUARTPrintStr(HAL_UART_PORT_0, "SYS TIMER: fail*\n");
  }
  SIM_Init();
}


/****  SYS STICK TIMER EVENT ***/
void ProcessStickTimerEvent(){
  curStickTime++;
  if (gw_IsStarted){
    HalLedBlink(HAL_LED_2, GW_DEFAULT_STICK_DURATION / 1000, 50, 1000);
  }
  /* for test */
 /* alivePacket.pktType = PKT_ALIVE_TYPE;
  alivePacket.pktPara.alivePara.curTime = curStickTime;
  alivePacket.pktPara.alivePara.nodeId  = 0;

  GW_SendDataRequest(PKT_ALIVE_TYPE, (uint8*) &(alivePacket.pktPara.alivePara),
                     sizeof(alivePara_t), gw_DevShortAddrList[gw_NumOfDevices-1]);
  */
}


/*** SCAN CONFIRM EVENT  ***/
void ProcessingScanConfirm(macCbackEvent_t * pData){
  if (pData->scanCnf.hdr.status == MAC_NO_BEACON){
    HalUARTPrintStr(HAL_UART_PORT_0, "SCAN: no beacon\n");
  }
  else if (pData->scanCnf.hdr.status == MAC_INVALID_PARAMETER){
    HalUARTPrintStr(HAL_UART_PORT_0, "SCAN: invalid\n");
  }
  else if (pData->scanCnf.hdr.status == MAC_SUCCESS){
    uint8 minCh = 0;     /* find minimum */
    uint8 minChEnergy = pData->scanCnf.result.pEnergyDetect[0];
    uint8 i;
    HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "CH[", 11, 10);
    HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "]: ", minChEnergy, 10);
    for (i=1; i<pData->scanCnf.resultListSize; i++){
      HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "  CH[", i+11, 10);
      HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "]: ", pData->scanCnf.result.pEnergyDetect[i], 10);
      if (minChEnergy > pData->scanCnf.result.pEnergyDetect[i]){
        minChEnergy = pData->scanCnf.result.pEnergyDetect[i];
        minCh = i;
      }
    }
    HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "\nCOOR start on: ", (minCh+11), 10);
    GW_CoordinatorStartup(minCh+11);
  }
}


/**** ASSOC IND event   ********************************/
void ProcessAssocIndEvent(macCbackEvent_t* pMsg){
  uint16 assocShortAddress;

  if (gw_NumOfDevices == GW_MAX_DEVICE_NUM){   /* RESET the association IF over numbers */
    gw_NumOfDevices = 0;
    /* Add code clear pending packet */
    HalUARTPrintStr(HAL_UART_PORT_0, "ASSOS: reset\n");
  }
  /* Build the record for this device */
  assocShortAddress = gw_DevShortAddrList[gw_NumOfDevices];
  gw_NumOfDevices++;
  /* Fill in association respond message */
  sAddrExtCpy(gw_AssocRsp.deviceAddress, pMsg->associateInd.deviceAddress);
  gw_AssocRsp.assocShortAddress  = assocShortAddress;
  gw_AssocRsp.status             = MAC_SUCCESS;
  gw_AssocRsp.sec.securityLevel  = MAC_SEC_LEVEL_NONE;
  /* Call Associate Response */
  MAC_MlmeAssociateRsp(&gw_AssocRsp);
  HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "ASSOS: allow ", assocShortAddress, 10);
}


/****   RECEIVING PACKET event   ***********************/
void ProcessReceivingPacket(macMcpsDataInd_t* pData){
  pkt_t*        recvPkt;

  HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "RECV: sAdd(", pData->mac.srcAddr.addr.shortAddr, 10);
  HalUARTPrintStrAndUInt(HAL_UART_PORT_0, ") time: ", curStickTime, 10);
  HalUARTPrintStrAndInt(HAL_UART_PORT_0, "  rssi: ", pData->mac.rssi, 10);
  HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "  linkQuality: ", pData->mac.mpduLinkQuality,10);

  recvPkt = (pkt_t*) pData->msdu.p;
  PKT_Print(recvPkt);
}

void GW_SendDataRequest(pktType_t pktType, uint8* pktPara, uint8 paraLen, uint16 dstShortAddr){
  macMcpsDataReq_t  *pData;
  static uint8      handle = 0;
  pkt_t *dstAppPkt;

  if ((pData = MAC_McpsDataAlloc(sizeof(pktType)+paraLen, MAC_SEC_LEVEL_NONE, MAC_KEY_ID_MODE_IMPLICIT)) != NULL){
    pData->mac.srcAddrMode            = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addrMode       = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addr.shortAddr = dstShortAddr;
    pData->mac.dstPanId               = gw_PanId;
    pData->mac.msduHandle             = handle++;
    pData->mac.txOptions              = MAC_TXOPTION_ACK | MAC_TXOPTION_INDIRECT;
    pData->sec.securityLevel          = MAC_SEC_LEVEL_NONE;
    dstAppPkt = (pkt_t*) pData->msdu.p;
    dstAppPkt->pktType = pktType;
    osal_memcpy(&(dstAppPkt->pktPara), pktPara, paraLen);
    HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "PEND: to ", dstShortAddr, 10);
    MAC_McpsDataReq(pData);
  }
  else{
    HalUARTPrintStr(HAL_UART_PORT_0, "PEND: mem deny\n");
  }
}


/**************************************************************************************************
 * @brief   Callback service for keys
 * @param   keys  - keys that were pressed
 *          state - shifted
 * @return  void
 **************************************************************************************************/
void GW_HandleKeys(uint8 keys, uint8 shift)
{
  if (keys & HAL_KEY_SW_1){

  }
  else if (keys & HAL_KEY_SW_2){

  }
  else if (keys & HAL_KEY_SW_3){

  }
  else if (keys & HAL_KEY_SW_1){

  }
}


void UART0Start(){
  halUARTCfg_t uartConfig;

  /* UART Configuration */
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = HAL_UART_BR_38400;
  uartConfig.flowControl          = HAL_UART_FLOW_OFF;
  uartConfig.flowControlThreshold = 16;
  uartConfig.rx.maxBufSize        = UART0_RX_BUF_SIZE;
  uartConfig.tx.maxBufSize        = UART0_TX_BUF_SIZE;
  uartConfig.idleTimeout          = 6;
  uartConfig.intEnable            = TRUE;
  uartConfig.callBackFunc         = GW_UARTCallBack;

  HalUARTOpen (HAL_UART_PORT_0, &uartConfig);
}


///////////////////////////////////////////////////////////
void GW_UARTCallBack (uint8 port, uint8 event){
    //do nothing now
}


/**************************************************************************************************
 * @brief       This callback function sends MAC events to the application.
 *              The application must implement this function.  A typical
 *              implementation of this function would allocate an OSAL message,
 *              copy the event parameters to the message, and send the message
 *              to the application's OSAL event handler.  This function may be
 *              executed from task or interrupt context and therefore must
 *              be reentrant.
 * @param       pData - Pointer to parameters structure.
 * @return      None.
 **************************************************************************************************/
void MAC_CbackEvent(macCbackEvent_t *pData){
  macCbackEvent_t *pMsg = NULL;

  uint8 len = gw_cbackSizeTable[pData->hdr.event];
  switch (pData->hdr.event){
    case MAC_MCPS_DATA_IND:
      pMsg = pData;
      break;

    default:
      if ((pMsg = (macCbackEvent_t *) osal_msg_allocate(len)) != NULL){
        osal_memcpy(pMsg, pData, len);
      }
      break;
  }

  if (pMsg != NULL){
    osal_msg_send(GW_TaskId, (uint8 *) pMsg);
  }
}


/**************************************************************************************************
 * @brief   Returns the number of indirect messages pending in the application
 * @param   None
 * @return  Number of indirect messages in the application
 **************************************************************************************************/
uint8 MAC_CbackCheckPending(void)
{
  return (0);
}

/**************************************************************************************************
 * @brief       This function callback function returns whether or not to continue MAC
 *              retransmission.
 *              A return value '0x00' will indicate no continuation of retry and a return value
 *              '0x01' will indicate to continue retransmission. This callback function shall be
 *              used to stop continuing retransmission for RF4CE.
 *              MAC shall call this callback function whenever it finishes transmitting a packet
 *              for macMaxFrameRetries times.
 * @return      0x00 to stop retransmission, 0x01 to continue retransmission.
 **************************************************************************************************
*/
uint8 MAC_CbackQueryRetransmit(void)
{
  /* Stub */
  return(0);
}