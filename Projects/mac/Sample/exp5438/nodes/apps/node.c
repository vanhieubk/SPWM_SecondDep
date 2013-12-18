/* Hal Driver includes */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
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
#include "node.h"
#include "nwk_comm.h"
#include "fram.h"
#include "sensing.h"
#include "packet.h"

/**** DEFINE   ****/
#define UART0_RX_BUF_SIZE         128
#define UART0_TX_BUF_SIZE         1024



#define NODE_DEV_SHORT_ADDR        0x0000        /* Device initial short address - This will change after association */
#define NODE_HEADER_LENGTH         4             /* Header includes DataLength + DeviceShortAddr + Sequence */
#define NODE_ECHO_LENGTH           8             /* Echo packet */

/* Size table for MAC structures */
const CODE uint8 node_cbackSizeTable [] =
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


/**** VARIABLEs  ****/
sAddrExt_t    node_ExtAddr = {0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x00, 0x00};
/* Coordinator and Device information */
uint16        node_PanId          = NWK_PAN_ID;
uint16        node_CoordShortAddr = NWK_GW_SHORT_ADDR;
uint16        node_DevShortAddr   = NODE_DEV_SHORT_ADDR;

/* TRUE and FALSE value */
bool          node_MACTrue  = TRUE;
bool          node_MACFalse = FALSE;
/* Beacon payload, this is used to determine if the device is not zigbee device */
uint8         node_BeaconPayload[]  = {0x22, 0x33, 0x44};
uint8         node_BeaconPayloadLen = 3;
/* Structure that used for association request */
macMlmeAssociateReq_t  node_AssociateReq;
/* Structure that used for association response */
macMlmeAssociateRsp_t  node_AssociateRsp;
uint8   node_SuperFrameOrder  = NWK_MAC_SUPERFRAME_ORDER;
uint8   node_BeaconOrder      = NWK_MAC_BEACON_ORDER;
int8    node_TxPower          = 0;
/* Task ID */
uint8   NODE_TaskId;
/* flags used in the application */
bool    isAssociated   = FALSE;
bool    isGWDetect     = FALSE;
bool    isPendData     = FALSE;
/* Retry rescan */
uint8   curRescanNum       = 0;
uint8   scanTimeOut        = NODE_DEFAULT_SCAN_TIME_OUT;
uint8   numRescanTry       = NODE_DEFAULT_NUM_RESCAN_TRY;
uint16  rescanWaitTime     = NODE_DEFAULT_RESCAN_WAIT_TIME;
/* Sensing time management */
uint16  stickDuration  = NODE_DEFAULT_STICK_DURATION;
uint16  preparingDelta = NODE_DEFAULT_PREPARING_DELTA;
uint16  sensingTime    = NODE_DEFAULT_SENSING_TIME;
uint16  sendAliveTime  = NODE_DEFAULT_SEND_ALIVE_TIME;
uint32  curStickTime   = NODE_DEFAULT_SENSING_TIME;
/* Sensing result */
sensing_t ssResult;

/**** FUNCTIONs ****/
void UART0Start(void);
void NODE_UARTCallBack (uint8 port, uint8 event);

void NODE_DeviceStartup(void);
/* MAC related routines */
void NODE_PollRequest(void);

/* Support */
void NODE_SendDirect(pktType_t pktType, uint8* pktPara, uint8 paraLen, uint16 dstShortAddr);
void NODE_SendAlive(void);
void NODE_SendSensingResult(void);

/* Process event / request */
void ProcessInitPeriEvent(void);
void ProcessScanConfirmEvent(macCbackEvent_t * pData);
void ProcessStickTimerEvent(void);
void ProcessScanFail(void);
void ProcessStickTimerEvent(void);
void ProcessAssocConfirmEvent(macCbackEvent_t *pData);

/**************************************************************************************************
 * @brief       Initialize the application
 * @param       taskId - taskId of the task after it was added in the OSAL task queue
 * @return      none
 **************************************************************************************************/
void NODE_Init(uint8 taskId){
  NODE_TaskId = taskId;     /* store taskId */
  MAC_InitDevice();         /* initialize MAC features */
   MAC_InitCoord();
  MAC_MlmeResetReq(TRUE);   /* Reset the MAC */

  UART0Start();             /* init UART0 for PC communication */
  osal_start_timerEx(NODE_TaskId, NODE_PREP_INIT_EVENT, 1000); /* delay 1s after power up */
}


/**************************************************************************************************
 * @brief       This routine handles events
 * @param       taskId - ID of the application task when it registered with the OSAL
 *              events - Events for this task
 * @return      16bit - Unprocessed events
 **************************************************************************************************/
uint16 NODE_ProcessEvent(uint8 taskId, uint16 events)
{
  uint8* pMsg;
  macCbackEvent_t* pData;

  if (events & SYS_EVENT_MSG){
    while ((pMsg = osal_msg_receive(NODE_TaskId)) != NULL){
      switch ( *pMsg ){
        case MAC_MLME_SCAN_CNF:
          pData = (macCbackEvent_t *) pMsg;
          ProcessScanConfirmEvent(pData);
          break;
        case MAC_MLME_ASSOCIATE_CNF:
          pData = (macCbackEvent_t *) pMsg; /* Retrieve the message */
          ProcessAssocConfirmEvent(pData);
          break;
        case MAC_MCPS_DATA_CNF:/* Send COMPLETED, success or fail or overflow ... */
          pData = (macCbackEvent_t *) pMsg;
          switch (pData->hdr.status){
          case MAC_SUCCESS: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: success\n"); break;
          case MAC_CHANNEL_ACCESS_FAILURE: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: access fail\n"); break;
          case MAC_FRAME_TOO_LONG: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: too long\n"); break;
          case MAC_INVALID_PARAMETER: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: invalid\n"); break;
          case MAC_NO_ACK: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: no ACK\n"); break;
          case MAC_TRANSACTION_EXPIRED: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: expired\n"); break;
          case MAC_TRANSACTION_OVERFLOW: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: overflow\n"); break;
          case MAC_COUNTER_ERROR: HalUARTPrintStr(HAL_UART_PORT_0, "SENT: error\n"); break;
          }
          mac_msg_deallocate((uint8**) &(pData->dataCnf.pDataReq));
          HalUARTPrintStr(HAL_UART_PORT_0, "DATA: sent\n");
          break;
      } /* end switch */
      //mac_msg_deallocate((uint8 **)&pMsg);       /* Deallocate */
      osal_msg_deallocate((uint8 *)&pMsg);
    } /* end while */
    return events ^ SYS_EVENT_MSG;
  } /* end sys event */

  if (events & NODE_PREP_INIT_EVENT){
    ProcessInitPeriEvent();
    return events ^ NODE_PREP_INIT_EVENT;
  }

  if (events & NODE_RESCAN_EVENT){
    NWK_ScanReq(MAC_SCAN_ACTIVE, scanTimeOut);
    HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "SCAN: rescan ", curRescanNum, 10);
    return events ^ NODE_RESCAN_EVENT;
  }

  if (events & NODE_STICK_TIMER_EVENT){
    ProcessStickTimerEvent();
    return events ^ NODE_STICK_TIMER_EVENT;
  }

  return 0;
}



void ProcessInitPeriEvent(void){
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
  NWK_ScanReq(MAC_SCAN_ACTIVE, scanTimeOut);

  SS_Init(); /* Init Sensing module */

  stickDuration  = NODE_DEFAULT_STICK_DURATION;
  curStickTime   = NODE_DEFAULT_SENSING_TIME;
  preparingDelta = NODE_DEFAULT_PREPARING_DELTA;
  sensingTime    = NODE_DEFAULT_SENSING_TIME;
  /* Start sensing timer */
  if (SUCCESS == osal_start_reload_timer(NODE_TaskId, NODE_STICK_TIMER_EVENT, stickDuration)){
    HalUARTPrintStr(HAL_UART_PORT_0, "TIMER: start\n");
  }
  else{
    HalUARTPrintStr(HAL_UART_PORT_0, "\n\n*********************\n");
    HalUARTPrintStr(HAL_UART_PORT_0, "* TIMER: fail, System halt *\n");
    HalUARTPrintStr(HAL_UART_PORT_0, "\n\n*********************\n");
  }
}

/********************************************/
void ProcessScanFail(){
  curRescanNum++;
  if (curRescanNum <= numRescanTry){
    osal_start_timerEx(NODE_TaskId, NODE_RESCAN_EVENT, rescanWaitTime); /* Set time for next rescan */
    HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "SCAN: wait ", rescanWaitTime, 10);
  }
  else{
    HalUARTPrintStr(HAL_UART_PORT_0, "SCAN: abort\n");
   }
}


/********************************************/
void ProcessScanConfirmEvent(macCbackEvent_t* pData){
  macMlmeScanCnf_t* scanCnf = &(pData->scanCnf);
  uint8 numResult, i;

  if (scanCnf->hdr.status == MAC_NO_BEACON){
    HalUARTPrintStr(HAL_UART_PORT_0, "SCAN: no beacon\n");
    ProcessScanFail();
  }
  else if (scanCnf->hdr.status == MAC_INVALID_PARAMETER){
    HalUARTPrintStr(HAL_UART_PORT_0, "SCAN: invalid\n");
    ProcessScanFail();
  }
  else if (MAC_SCAN_ACTIVE == scanCnf->scanType){
    numResult = scanCnf->resultListSize;
    HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "SCAN result: ", numResult, 10);
    for (i=0; i<numResult; i++){
      HalUARTPrintStrAndUInt(HAL_UART_PORT_0, "COOR: ", scanResult.panDesc[i].coordPanId, 16);
      HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "  channel: ", scanResult.panDesc[i].logicalChannel, 10);

      if (NWK_PAN_ID == scanResult.panDesc[i].coordPanId){
        isGWDetect = TRUE;
        node_AssociateReq.logicalChannel = scanResult.panDesc[i].logicalChannel;
        node_AssociateReq.channelPage    = scanResult.panDesc[i].channelPage;
        node_AssociateReq.coordAddress.addrMode       = SADDR_MODE_SHORT;
        node_AssociateReq.coordAddress.addr.shortAddr = scanResult.panDesc[i].coordAddress.addr.shortAddr;
        node_AssociateReq.coordPanId            = NWK_PAN_ID;
        node_AssociateReq.capabilityInformation = MAC_CAPABLE_ALLOC_ADDR;
        node_AssociateReq.sec.securityLevel     = MAC_SEC_LEVEL_NONE;

        /* MUST after setting node_AssosciateReq */
        NODE_DeviceStartup(); /* Start the devive up */
        HalUARTPrintStr(HAL_UART_PORT_0, "ASSOC: request\n");
        MAC_MlmeAssociateReq(&node_AssociateReq);  /* Call Associate Req */
      }
    }
    if (FALSE == isGWDetect){ /* not DETECT GateWay */
      HalUARTPrintStr(HAL_UART_PORT_0, "SCAN (active): no GW\n");
      ProcessScanFail();
    }
  }
  else if (MAC_SCAN_ORPHAN== scanCnf->scanType){
    HalUARTPrintStr(HAL_UART_PORT_0, "Orphan scan\n");
  }
}


/***************************/
void ProcessAssocConfirmEvent(macCbackEvent_t *pData){
  if ((!isAssociated) && (pData->associateCnf.hdr.status == MAC_SUCCESS)){
    isAssociated = TRUE;
    node_DevShortAddr = pData->associateCnf.assocShortAddress; /* Retrieve MAC_SHORT_ADDRESS */
    MAC_MlmeSetReq(MAC_SHORT_ADDRESS, &node_DevShortAddr); /* Setup MAC_SHORT_ADDRESS - obtained from Association */
    HalUARTPrintStr(HAL_UART_PORT_0, "ASSOC: OK\n");
    HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "ADDRESS: ", node_DevShortAddr, 16);
    if (isPendData){
      NODE_SendSensingResult();
      isPendData = FALSE;
    }
  }
  else{
    /* IF COORDINATOR deny association, SNs will be exhaused energy for try assoc */
    ProcessScanFail(); /* Treat as scan fail */
  }
}


void ProcessStickTimerEvent(){
  curStickTime++;
  HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "TIMER: fire ", curStickTime, 10);
  if (0 == (curStickTime % sensingTime)){
    HalUARTPrintStr(HAL_UART_PORT_0,"\nSENING: sensing\n");
    SS_Measure(&ssResult);
    SS_Shutdown();
    if (isAssociated){
      HalUARTPrintStr(HAL_UART_PORT_0, "SEND: senResult\n");
      NODE_SendSensingResult();
    }
    else{
      isPendData = TRUE;
      HalUARTPrintStr(HAL_UART_PORT_0, "PEND: senResult\nSCAN: restart\n");
      curRescanNum = 0;
      NWK_ScanReq(MAC_SCAN_ACTIVE, scanTimeOut);
    }
    SS_Print(&ssResult); /* out result for debug */
  }
  else{
    if (0 == ((curStickTime-preparingDelta) % sensingTime)){
      HalUARTPrintStr(HAL_UART_PORT_0,"SENING: prepare\n");
      SS_Prepare();
    }
    if (0 == (curStickTime % sendAliveTime)){
      if (isAssociated){
        NODE_SendAlive();
        HalUARTPrintStr(HAL_UART_PORT_0, "SEND: alive\n");
      }
      else{
        HalUARTPrintStr(HAL_UART_PORT_0, "PEND: alive\n");
      }
    }
  }
}
/**************************************************************************************************
 * @brief   Update the timer per tick
 * @param   beaconEnable: TRUE/FALSE
 * @return  None
 **************************************************************************************************/
void NODE_DeviceStartup()
{
  uint16 AtoD = 0;
  AtoD = MAC_RADIO_RANDOM_WORD();
  AtoD = macMcuPrecisionCount();
  node_ExtAddr[6] = HI_UINT16( AtoD );
  node_ExtAddr[7] = LO_UINT16( AtoD );

  /* Setup Ext address */
  MAC_MlmeSetReq(MAC_EXTENDED_ADDRESS, &node_ExtAddr);
  /* Setup MAC_BEACON_PAYLOAD_LENGTH */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD_LENGTH, &node_BeaconPayloadLen);
  /* Setup MAC_BEACON_PAYLOAD */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD, &node_BeaconPayload);
  /* Setup PAN ID */
  MAC_MlmeSetReq(MAC_PAN_ID, &node_PanId);
  /* This device is setup for Direct Message */
  MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &node_MACFalse);
  MAC_MlmeSetReq(MAC_SECURITY_ENABLED, &node_MACFalse);
  MAC_MlmeSetReq(MAC_PHY_TRANSMIT_POWER_SIGNED, &node_TxPower);
  /* Setup Coordinator short address */
  MAC_MlmeSetReq(MAC_COORD_SHORT_ADDRESS, &node_AssociateReq.coordAddress.addr.shortAddr);
  node_CoordShortAddr = node_AssociateReq.coordAddress.addr.shortAddr;
  HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "COOR: addr ", node_CoordShortAddr, 16);
  /* Power saving */
  NODE_PowerMgr (NODE_PWR_MGMT_ENABLED);
}


void NODE_SendDirect(pktType_t pktType, uint8* pktPara, uint8 paraLen, uint16 dstShortAddr){
  static uint8 msduHandle=0;
  macMcpsDataReq_t*  sentMACPkt    = NULL;
  pkt_t *dstAppPkt;

  sentMACPkt = MAC_McpsDataAlloc(sizeof(pktType_t)+paraLen, MAC_SEC_LEVEL_NONE, MAC_KEY_ID_MODE_IMPLICIT );
  if ((NULL == sentMACPkt)){
    HalUARTPrintStr(HAL_UART_PORT_0, "MEM: deny\n");
    return;
  }

  sentMACPkt->mac.srcAddrMode            = SADDR_MODE_SHORT;
  sentMACPkt->mac.dstAddr.addrMode       = SADDR_MODE_SHORT;
  sentMACPkt->mac.dstAddr.addr.shortAddr = dstShortAddr;
  sentMACPkt->mac.dstPanId               = node_PanId;
  sentMACPkt->mac.txOptions              = MAC_TXOPTION_NO_RETRANS;
  //sentMACPkt->mac.channel                = 11;
  //sentMACPkt->mac.power                  = 0;
  sentMACPkt->sec.securityLevel          = MAC_SEC_LEVEL_NONE;
  //sentMACPkt->sec.keyIdMode              = MAC_KEY_ID_MODE_NONE;
  sentMACPkt->mac.msduHandle = msduHandle++;
  //sentMACPkt->msdu.len = sizeof(pktType_t)+paraLen;
  dstAppPkt = (pkt_t*) sentMACPkt->msdu.p;
  dstAppPkt->pktType = pktType;
  osal_memcpy(&(dstAppPkt->pktPara), pktPara, paraLen);
  MAC_McpsDataReq(sentMACPkt);
}

void NODE_SendAlive(void){
  NODE_SendDirect(PKT_ALIVE_TYPE, (uint8*) &curStickTime, sizeof(curStickTime), node_CoordShortAddr);
}


void NODE_SendSensingResult(void){
  NODE_SendDirect(PKT_SENSING_TYPE, (uint8*) &ssResult, sizeof(ssResult), node_CoordShortAddr);
}

/**************************************************************************************************
 * @brief   Performs a poll request on the coordinator
 * @param   None
 * @return  None
 **************************************************************************************************/
void NODE_PollRequest(void)
{
  macMlmePollReq_t  pollReq;

  /* Fill in information for poll request */
  pollReq.coordAddress.addrMode       = SADDR_MODE_SHORT;
  pollReq.coordAddress.addr.shortAddr = node_CoordShortAddr;
  pollReq.coordPanId        = node_PanId;
  pollReq.sec.securityLevel = MAC_SEC_LEVEL_NONE;
  /* Call poll request */
  MAC_MlmePollReq(&pollReq);
}


/**************************************************************************************************
 * @brief   Callback service for keys
 * @param   keys  - keys that were pressed
 *          state - shifted
 * @return  void
 **************************************************************************************************/
void NODE_HandleKeys(uint8 keys, uint8 shift)
{
  if (keys & HAL_KEY_SW_1){
    HalLedSet(HAL_LED_2, HAL_LED_MODE_ON);
    HalUARTPrintnlStr(HAL_UART_PORT_0, "HELLO");
  }
  else if (keys & HAL_KEY_SW_2){
    HalLedSet(HAL_LED_2, HAL_LED_MODE_OFF);
    HalUARTPrintnlStrAndUInt(HAL_UART_PORT_0, "UInt", 23524563, 16);
  }
  else if (keys & HAL_KEY_SW_3){
    HalLedSet(HAL_LED_2, HAL_LED_MODE_TOGGLE);
    HalUARTPrintnlStrAndInt(HAL_UART_PORT_0, "Int", -123456789, 10);
  }
  else if (keys & HAL_KEY_SW_1){
    HalLedBlink(HAL_LED_2, 0, 50, 200);
  }
}


/***********************************************/
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
  uartConfig.callBackFunc         = NODE_UARTCallBack;

  HalUARTOpen (HAL_UART_PORT_0, &uartConfig);
}


void NODE_UARTCallBack (uint8 port, uint8 event)
{
    //do nothing now
}


/**
 * @brief       This callback function sends MAC events to the application.
 *              The application must implement this function.  A typical
 *              implementation of this function would allocate an OSAL message,
 *              copy the event parameters to the message, and send the message
 *              to the application's OSAL event handler.  This function may be
 *              executed from task or interrupt context and therefore must
 *              be reentrant.
 * @return      None.
 */
void MAC_CbackEvent(macCbackEvent_t *pData)
{
  macCbackEvent_t *pMsg = NULL;

  uint8 len = node_cbackSizeTable[pData->hdr.event];
  switch (pData->hdr.event)
  {
    case MAC_MCPS_DATA_IND:
      pMsg = pData;
      break;
    default:
      if ((pMsg = (macCbackEvent_t *) osal_msg_allocate(len)) != NULL) {
        osal_memcpy(pMsg, pData, len);
      }
      break;
  }
  if (pMsg != NULL) {
    osal_msg_send(NODE_TaskId, (uint8 *) pMsg);
  }
}

/**
 * @brief   Returns the number of indirect messages pending in the application
 * @return  Number of indirect messages in the application
 */
uint8 MAC_CbackCheckPending(void)
{
  return (0);
}

/**
 * @brief       This function callback function returns whether or not to continue MAC
 *              retransmission.
 *              A return value '0x00' will indicate no continuation of retry and a return value
 *              '0x01' will indicate to continue retransmission. This callback function shall be
 *              used to stop continuing retransmission for RF4CE.
 *              MAC shall call this callback function whenever it finishes transmitting a packet
 *              for macMaxFrameRetries times.
 * @return      0x00 to stop retransmission, 0x01 to continue retransmission.
*/
uint8 MAC_CbackQueryRetransmit(void)
{
  /* Stub */
  return(0);
}
