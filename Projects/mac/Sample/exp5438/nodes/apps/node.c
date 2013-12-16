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

/**** DEFINE   ****/
#define UART0_RX_BUF_SIZE         128
#define UART0_TX_BUF_SIZE         512
void UART0Start(void);


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

/**************************************************************************************************
 *                                        Local Variables
 **************************************************************************************************/
sAddrExt_t    node_ExtAddr = {0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, 0x00, 0x00};

/* Coordinator and Device information */
uint16        node_PanId = NWK_PAN_ID;
uint16        node_CoordShortAddr = NWK_GW_SHORT_ADDR;
uint16        node_DevShortAddr   = NODE_DEV_SHORT_ADDR;

/* Predefined packet that will be sent out by the coordinator */
uint8         node_Data1[NODE_PACKET_LENGTH];

/* Predefined packet that will be sent back to the coordinator by the device */
uint8         node_Data2[NODE_ECHO_LENGTH];

/* TRUE and FALSE value */
bool          node_MACTrue = TRUE;
bool          node_MACFalse = FALSE;

/* Beacon payload, this is used to determine if the device is not zigbee device */
uint8         node_BeaconPayload[] = {0x22, 0x33, 0x44};
uint8         node_BeaconPayloadLen = 3;


/* flags used in the application */
bool          node_IsCoordinator  = FALSE;   /* True if the device is started as a Pan Coordinate */
bool          node_IsStarted      = FALSE;   /* True if the device started, either as Pan Coordinator or device */
bool          node_IsSampleBeacon = FALSE;   /* True if the beacon payload match with the predefined */
bool          node_IsDirectMsg    = NWK_DIRECT_MSG_ENABLED;   /* True if the messages will be sent as direct messages */
uint8         node_State = NODE_IDLE_STATE;   /* Either IDLE state or SEND state */

/* Structure that used for association request */
macMlmeAssociateReq_t node_AssociateReq;

/* Structure that used for association response */
macMlmeAssociateRsp_t node_AssociateRsp;
uint8 node_SuperFrameOrder;
uint8 node_BeaconOrder;

/* Task ID */
uint8 NODE_TaskId;


uint8 node_securityLevel = MAC_SEC_LEVEL_NONE;
uint8 node_keyIdMode     = MAC_KEY_ID_MODE_NONE;
uint8 node_keySource[]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
uint8 node_keyIndex      = 0;


/**************************************************************************************************
 *                                     Local Function Prototypes
 **************************************************************************************************/
void NODE_UARTCallBack (uint8 port, uint8 event);
/* Setup routines */
void NODE_DeviceStartup(void);

/* MAC related routines */
void NODE_AssociateReq(void);
void NODE_McpsDataReq(uint8* data, uint8 dataLength, bool directMsg, uint16 dstShortAddr);
void NODE_McpsPollReq(void);

/* Support */
bool NODE_DataCheck(uint8* data, uint8 dataLength);

/**************************************************************************************************
 *
 * @fn          NODE_Init
 *
 * @brief       Initialize the application
 *
 * @param       taskId - taskId of the task after it was added in the OSAL task queue
 *
 * @return      none
 *
 **************************************************************************************************/
void NODE_Init(uint8 taskId)
{
  uint8 i;

  /* Initialize the task id */
  NODE_TaskId = taskId;

  /* initialize MAC features */
  //MAC_InitDevice();
  MAC_InitCoord();

  /* Initialize MAC beacon */
  //MAC_InitBeaconDevice();
  //MAC_InitBeaconCoord();

  /* Reset the MAC */
  MAC_MlmeResetReq(TRUE);

#ifdef FEATURE_MAC_SECURITY
  /* Initialize the security part of the application */
  NODE_SecurityInit();
#endif /* FEATURE_MAC_SECURITY */

  /* Initialize the data packet */
  for (i=NODE_HEADER_LENGTH; i<NODE_PACKET_LENGTH; i++)
  {
    node_Data1[i] = i-NODE_HEADER_LENGTH;
  }

  /* Initialize the echo packet */
  for (i=0; i<NODE_ECHO_LENGTH; i++)
  {
    node_Data2[i] = 0xEE;
  }

  node_BeaconOrder      = NWK_MAC_BEACON_ORDER;
  node_SuperFrameOrder  = NWK_MAC_SUPERFRAME_ORDER;

  /* init UART0 for PC communication */
  UART0Start();



  /* Start scan */
  HalUARTPrintStr(HAL_UART_PORT_0, "Start scan\n");
  NWK_ScanReq(MAC_SCAN_ACTIVE, 3);
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

#ifdef FEATURE_MAC_SECURITY
  uint16       panID;
  uint16       panCoordShort;
  sAddrExt_t   panCoordExtAddr;
#endif /* FEATURE_MAC_SECURITY */

  static uint8 index;
  static uint8 sequence;

  if (events & SYS_EVENT_MSG)
  {
    while ((pMsg = osal_msg_receive(NODE_TaskId)) != NULL)
    {
      switch ( *pMsg )
      {
        case MAC_MLME_ASSOCIATE_CNF:
          /* Retrieve the message */
          pData = (macCbackEvent_t *) pMsg;

          if ((!node_IsStarted) && (pData->associateCnf.hdr.status == MAC_SUCCESS))
          {
            node_IsStarted = TRUE;
            /* Retrieve MAC_SHORT_ADDRESS */
            node_DevShortAddr = pData->associateCnf.assocShortAddress;

            /* Setup MAC_SHORT_ADDRESS - obtained from Association */
            MAC_MlmeSetReq(MAC_SHORT_ADDRESS, &node_DevShortAddr);

            HalLedBlink(HAL_LED_4, 0, 90, 1000);

            /* Poll for data if it's not setup for direct messaging */
            if (!node_IsDirectMsg)
            {
              //osal_start_timerEx(NODE_TaskId, NODE_POLL_EVENT, NODE_WAIT_PERIOD);
            }
          }
          break;

        case MAC_MLME_COMM_STATUS_IND:
          break;

        case MAC_MLME_START_CNF:
          /* Retrieve the message */
          pData = (macCbackEvent_t *) pMsg;
          /* Set some indicator for the Coordinator */
          if ((!node_IsStarted) && (pData->startCnf.hdr.status == MAC_SUCCESS))
          {
            node_IsStarted = TRUE;
            node_IsCoordinator = TRUE;
            HalLedSet (HAL_LED_4, HAL_LED_MODE_ON);
          }
          break;

        case MAC_MLME_SCAN_CNF:
          /* Check if there is any Coordinator out there */
          pData = (macCbackEvent_t *) pMsg;

          switch (pData->scanCnf.scanType){
            case MAC_SCAN_ED: HalUARTPrintStr(HAL_UART_PORT_0, "Energy scan\n"); break;
            case MAC_SCAN_PASSIVE: HalUARTPrintStr(HAL_UART_PORT_0, "Passive scan\n"); break;
            case MAC_SCAN_ACTIVE: HalUARTPrintStr(HAL_UART_PORT_0, "Active scan\n"); break;
            case MAC_SCAN_ORPHAN: HalUARTPrintStr(HAL_UART_PORT_0, "Orphan scan\n"); break;
          }
          if ((node_IsSampleBeacon) && pData->scanCnf.hdr.status == MAC_SUCCESS)
          {
            /* Start the devive up as beacon enabled or not */
            NODE_DeviceStartup();

            /* Call Associate Req */
            NODE_AssociateReq();
          }
          break;

        case MAC_MCPS_DATA_CNF:
          pData = (macCbackEvent_t *) pMsg;

          /* If last transmission completed, ready to send the next one */
          if ((pData->dataCnf.hdr.status == MAC_SUCCESS) ||
              (pData->dataCnf.hdr.status == MAC_CHANNEL_ACCESS_FAILURE) ||
              (pData->dataCnf.hdr.status == MAC_NO_ACK))
          {
            //osal_start_timerEx(NODE_TaskId, NODE_SEND_EVENT, NODE_WAIT_PERIOD);
          }

          mac_msg_deallocate((uint8**)&pData->dataCnf.pDataReq);
          break;

        case MAC_MCPS_DATA_IND:
          pData = (macCbackEvent_t*)pMsg;

          if (NODE_DataCheck ( pData->dataInd.msdu.p, pData->dataInd.msdu.len ))
          {
            HalLedSet (HAL_LED_3, HAL_LED_MODE_TOGGLE);

            /* Only send the echo back if the received */
            if (!node_IsCoordinator)
            {
              if (NODE_ECHO_LENGTH >= 4)
              {
                /* Return the first 4 bytes and the last byte of the received packet */
                node_Data2[0] = pData->dataInd.msdu.p[0];
                node_Data2[1] = pData->dataInd.msdu.p[1];
                node_Data2[2] = pData->dataInd.msdu.p[2];
                node_Data2[3] = pData->dataInd.msdu.p[3];
              }

              NODE_McpsDataReq(node_Data2,
                              NODE_ECHO_LENGTH,
                              TRUE,
                              node_CoordShortAddr );
            }
          }
          break;
      }

      /* Deallocate */
      mac_msg_deallocate((uint8 **)&pMsg);
    }

    return events ^ SYS_EVENT_MSG;
  }

  /* This event handles polling for in-direct message device */
  if (events & NODE_POLL_EVENT)
  {
    NODE_McpsPollReq();
    //osal_start_timerEx(NODE_TaskId, NODE_POLL_EVENT, NODE_WAIT_PERIOD);

    return events ^ NODE_POLL_EVENT;
  }
  return 0;
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
void MAC_CbackEvent(macCbackEvent_t *pData)
{

  macCbackEvent_t *pMsg = NULL;

  uint8 len = node_cbackSizeTable[pData->hdr.event];

  switch (pData->hdr.event)
  {
      case MAC_MLME_BEACON_NOTIFY_IND:

      len += sizeof(macPanDesc_t) + pData->beaconNotifyInd.sduLength +
             MAC_PEND_FIELDS_LEN(pData->beaconNotifyInd.pendAddrSpec);
      if ((pMsg = (macCbackEvent_t *) osal_msg_allocate(len)) != NULL)
      {
        /* Copy data over and pass them up */
        osal_memcpy(pMsg, pData, sizeof(macMlmeBeaconNotifyInd_t));
        pMsg->beaconNotifyInd.pPanDesc = (macPanDesc_t *) ((uint8 *) pMsg + sizeof(macMlmeBeaconNotifyInd_t));
        osal_memcpy(pMsg->beaconNotifyInd.pPanDesc, pData->beaconNotifyInd.pPanDesc, sizeof(macPanDesc_t));
        pMsg->beaconNotifyInd.pSdu = (uint8 *) (pMsg->beaconNotifyInd.pPanDesc + 1);
        osal_memcpy(pMsg->beaconNotifyInd.pSdu, pData->beaconNotifyInd.pSdu, pData->beaconNotifyInd.sduLength);
      }
      break;

    case MAC_MCPS_DATA_IND:
      pMsg = pData;
      break;

    default:
      if ((pMsg = (macCbackEvent_t *) osal_msg_allocate(len)) != NULL)
      {
        osal_memcpy(pMsg, pData, len);
      }
      break;
  }

  if (pMsg != NULL)
  {
    osal_msg_send(NODE_TaskId, (uint8 *) pMsg);
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
 * @fn          MAC_CbackQueryRetransmit
 *
 * @brief       This function callback function returns whether or not to continue MAC
 *              retransmission.
 *              A return value '0x00' will indicate no continuation of retry and a return value
 *              '0x01' will indicate to continue retransmission. This callback function shall be
 *              used to stop continuing retransmission for RF4CE.
 *              MAC shall call this callback function whenever it finishes transmitting a packet
 *              for macMaxFrameRetries times.
 *
 * input parameters
 *
 * None.
 *
 * output parameters
 *
 * None.
 *
 * @return      0x00 to stop retransmission, 0x01 to continue retransmission.
 **************************************************************************************************
*/
uint8 MAC_CbackQueryRetransmit(void)
{
  /* Stub */
  return(0);
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

  MAC_MlmeSetReq(MAC_EXTENDED_ADDRESS, &node_ExtAddr);
  /* Setup MAC_BEACON_PAYLOAD_LENGTH */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD_LENGTH, &node_BeaconPayloadLen);
  /* Setup MAC_BEACON_PAYLOAD */
  MAC_MlmeSetReq(MAC_BEACON_PAYLOAD, &node_BeaconPayload);
  /* Setup PAN ID */
  MAC_MlmeSetReq(MAC_PAN_ID, &node_PanId);
  /* This device is setup for Direct Message */
  if (node_IsDirectMsg){
    MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &node_MACTrue);
  }
  else{
    MAC_MlmeSetReq(MAC_RX_ON_WHEN_IDLE, &node_MACFalse);
  }
  /* Setup Coordinator short address */
  MAC_MlmeSetReq(MAC_COORD_SHORT_ADDRESS, &node_AssociateReq.coordAddress.addr.shortAddr);
  /* Power saving */
  NODE_PowerMgr (NODE_PWR_MGMT_ENABLED);
}

/**************************************************************************************************
 * @brief
 * @param    None
 * @return  None
 **************************************************************************************************/
void NODE_AssociateReq(void){
  MAC_MlmeAssociateReq(&node_AssociateReq);
}


/**************************************************************************************************
 * @brief   This routine calls the Data Request
 * @param   data       - contains the data that would be sent
 *          dataLength - length of the data that will be sent
 * @return  None
 **************************************************************************************************/
void NODE_McpsDataReq(uint8* data, uint8 dataLength, bool directMsg, uint16 dstShortAddr)
{
  macMcpsDataReq_t  *pData;
  static uint8      handle = 0;

  if ((pData = MAC_McpsDataAlloc(dataLength, node_securityLevel, node_keyIdMode)) != NULL)
  {
    pData->mac.srcAddrMode = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addrMode = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addr.shortAddr = dstShortAddr;
    pData->mac.dstPanId = node_PanId;
    pData->mac.msduHandle = handle++;
    pData->mac.txOptions = MAC_TXOPTION_ACK;

    /* MAC security parameters */
    osal_memcpy( pData->sec.keySource, node_keySource, MAC_KEY_SOURCE_MAX_LEN );
    pData->sec.securityLevel = node_securityLevel;
    pData->sec.keyIdMode = node_keyIdMode;
    pData->sec.keyIndex = node_keyIndex;

    /* If it's the coordinator and the device is in-direct message */
    if (node_IsCoordinator)
    {
      if (!directMsg)
      {
        pData->mac.txOptions |= MAC_TXOPTION_INDIRECT;
      }
    }

    /* Copy data */
    osal_memcpy (pData->msdu.p, data, dataLength);

    /* Send out data request */
    MAC_McpsDataReq(pData);
  }
}

/**************************************************************************************************
 * @brief   Performs a poll request on the coordinator
 * @param   None
 * @return  None
 **************************************************************************************************/
void NODE_McpsPollReq(void)
{
  macMlmePollReq_t  pollReq;

  /* Fill in information for poll request */
  pollReq.coordAddress.addrMode = SADDR_MODE_SHORT;
  pollReq.coordAddress.addr.shortAddr = node_CoordShortAddr;
  pollReq.coordPanId = node_PanId;
  pollReq.sec.securityLevel = MAC_SEC_LEVEL_NONE;

  /* Call poll reuqest */
  MAC_MlmePollReq(&pollReq);
}

/**************************************************************************************************
 * @brief   Check if the data match with the predefined data
 * @param    data - pointer to the buffer where the data will be checked against the predefined data
 *           dataLength - length of the data
 * @return  TRUE if the data matched else it's the response / echo packet
 **************************************************************************************************/
bool NODE_DataCheck(uint8* data, uint8 dataLength)
{
  uint8 i = 0;

  if (data[0] == dataLength)
  {
    for (i=NODE_HEADER_LENGTH; i<(data[0] - 1); i++)
    {
       if (data[i] != node_Data1[i])
         return FALSE;
    }
  }
  else
  {
    return FALSE;
  }
  return TRUE;
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


#define UART0_RX_BUF_SIZE         128
#define UART0_TX_BUF_SIZE         512
void UART0Start(){
  halUARTCfg_t uartConfig;

  /* UART Configuration */
  uartConfig.configured           = TRUE;
  uartConfig.baudRate             = HAL_UART_BR_9600;
  uartConfig.flowControl          = HAL_UART_FLOW_OFF;
  uartConfig.flowControlThreshold = 16;
  uartConfig.rx.maxBufSize        = UART0_RX_BUF_SIZE;
  uartConfig.tx.maxBufSize        = UART0_TX_BUF_SIZE;
  uartConfig.idleTimeout          = 6;
  uartConfig.intEnable            = TRUE;
  uartConfig.callBackFunc         = NODE_UARTCallBack;

  HalUARTOpen (HAL_UART_PORT_0, &uartConfig);
}


///////////////////////////////////////////////////////////
void NODE_UARTCallBack (uint8 port, uint8 event){
    //do nothing now
}