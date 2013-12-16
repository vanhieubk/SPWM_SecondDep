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

/**** DEFINE   ****/
#define UART0_RX_BUF_SIZE        128
#define UART0_TX_BUF_SIZE        512

#define GW_HEADER_LENGTH         4             /* Header includes DataLength + DeviceShortAddr + Sequence */
#define GW_ECHO_LENGTH           8             /* Echo packet */
#define GW_MAX_DEVICE_NUM        50            /* Maximun number of devices can associate with the coordinator */


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
/* Coordinator and Device information */
sAddrExt_t    gw_ExtAddr         = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88};
uint16        gw_PanId           = NWK_PAN_ID;
uint16        gw_CoordShortAddr  = NWK_GW_SHORT_ADDR;

/* List of short addresses that the coordinator will use to assign
   to each the device that associates to it */
uint16        gw_DevShortAddrList[] = {0x0001, 0x0002, 0x0003, 0x0004, 0x0005,
                                            0x0006, 0x0007, 0x0008, 0x0009, 0x000A,
                                            0x000B, 0x000C, 0x000D, 0x000E, 0x000F,
                                            0x0010, 0x0011, 0x0012, 0x0013, 0x0014,
                                            0x0015, 0x0016, 0x0017, 0x0018, 0x0019,
                                            0x001A, 0x001B, 0x001C, 0x001D, 0x001E,
                                            0x001F, 0x0020, 0x0021, 0x0022, 0x0023,
                                            0x0024, 0x0025, 0x0026, 0x0027, 0x0028,
                                            0x0029, 0x002A, 0x002B, 0x002C, 0x002D,
                                            0x002E, 0x002F, 0x0030, 0x0031, 0x0032};
uint8         gw_NumOfDevices = 0; /* Current number of devices associated to the coordinator */
uint8         gw_Data1[NWK_PACKET_LENGTH]; /* Predefined packet that will be sent out by the coordinator */
/* Predefined packet that will be sent back to the coordinator by the device */
uint8         gw_Data2[GW_ECHO_LENGTH];

/* TRUE and FALSE value */
bool          gw_MACTrue = TRUE;
bool          gw_MACFalse = FALSE;

/* flags used in the application */
bool          gw_IsStarted      = FALSE;   /* True if the device started, either as Pan Coordinator or device */
bool          gw_IsDirectMsg    = NWK_DIRECT_MSG_ENABLED;   /* True if the messages will be sent as direct messages */
uint8         gw_State          = GW_IDLE_STATE;   /* Either IDLE state or SEND state */

/* Structure that used for association request */
macMlmeAssociateReq_t gw_AssociateReq;
/* Structure that used for association response */
macMlmeAssociateRsp_t gw_AssociateRsp;

/* Structure that contains information about the device that associates with the coordinator */
typedef struct
{
  uint16 devShortAddr;
  uint8  isDirectMsg;
} gw_DeviceInfo_t;

/* Array contains the information of the devices */
static gw_DeviceInfo_t gw_DeviceRecord[GW_MAX_DEVICE_NUM];
uint8 gw_SuperFrameOrder;
uint8 gw_BeaconOrder;

/* Task ID */
uint8 GW_TaskId;



/**** LOCAL FUNCTIONs DECLARATION ****/
void UART0Start(void);
void Gateway_UARTCallBack (uint8 port, uint8 event);
/* Setup routines */
void GW_Startup(void);
/* MAC related routines */
void GW_AssociateRsp(macCbackEvent_t* pMsg);
void GW_McpsDataReq(uint8* data, uint8 dataLength, bool directMsg, uint16 dstShortAddr);
/* Support */
bool GW_DataCheck(uint8* data, uint8 dataLength);


/**************************************************************************************************
 * @brief       Initialize the GATEWAY
 * @param       taskId - taskId of the task after it was added in the OSAL task queue
 * @return      none
 **************************************************************************************************/
void GW_Init(uint8 taskId)
{
  uint8 i;

  /* Initialize the task id */
  GW_TaskId = taskId;
  /* initialize MAC features */
  MAC_InitDevice();
  MAC_InitCoord();
  /* Reset the MAC */
  MAC_MlmeResetReq(TRUE);
  /* Initialize the data packet */
  for (i=GW_HEADER_LENGTH; i<NWK_PACKET_LENGTH; i++)
  {
    gw_Data1[i] = i-GW_HEADER_LENGTH;
  }

  /* Initialize the echo packet */
  for (i=0; i<GW_ECHO_LENGTH; i++)
  {
    gw_Data2[i] = 0xEE;
  }

  gw_BeaconOrder     = NWK_MAC_BEACON_ORDER;
  gw_SuperFrameOrder = NWK_MAC_SUPERFRAME_ORDER;

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
uint16 GW_ProcessEvent(uint8 taskId, uint16 events)
{
  uint8* pMsg;
  macCbackEvent_t* pData;

  if (events & SYS_EVENT_MSG)
  {
    while ((pMsg = osal_msg_receive(GW_TaskId)) != NULL)
    {
      switch ( *pMsg )
      {
        /* NETWORK SCAN COMPLETED */
        case MAC_MLME_SCAN_CNF:
          pData = (macCbackEvent_t *) pMsg;
          switch (pData->scanCnf.scanType){
            case MAC_SCAN_ED: HalUARTPrintStr(HAL_UART_PORT_0, "Energy scan\n"); break;
            case MAC_SCAN_PASSIVE: HalUARTPrintStr(HAL_UART_PORT_0, "Passive scan\n"); break;
            case MAC_SCAN_ACTIVE: HalUARTPrintStr(HAL_UART_PORT_0, "Active scan\n"); break;
            case MAC_SCAN_ORPHAN: HalUARTPrintStr(HAL_UART_PORT_0, "Orphan scan\n"); break;
          }
          /* Check if there is any Coordinator out there */
          if (pData->scanCnf.resultListSize == 0)
          {
            GW_Startup();
          }
          break;

        /* COORDINATOR STARTED COMPLETED */
        case MAC_MLME_START_CNF:
          pData = (macCbackEvent_t *) pMsg;
          if (pData->startCnf.hdr.status == MAC_SUCCESS){
            gw_IsStarted       = TRUE;
            HalUARTPrintStr(HAL_UART_PORT_0, "COOR Started\n");
            HalLedSet(HAL_LED_4, HAL_LED_MODE_ON);
          }
          else{
            /* Add re-scan here */
          }
          break;

        /* HAS one DEVICE want ASSOCIATE */
        case MAC_MLME_ASSOCIATE_IND:
          GW_AssociateRsp((macCbackEvent_t*)pMsg);
          break;

        /* Communication status INDICATION */
        case MAC_MLME_COMM_STATUS_IND:
          /* asso OK, do nothing now */
          break;

        /* MAC send data RESPONSE */
        case MAC_MCPS_DATA_CNF:
          pData = (macCbackEvent_t *) pMsg;
          /* If last transmission completed, ready to send the next one */
          HalUARTPrintStr(HAL_UART_PORT_0, "Data response\n");

          /* ADD CODE PROCESSING HERE */
         /* if ((pData->dataCnf.hdr.status == MAC_SUCCESS) ||
              (pData->dataCnf.hdr.status == MAC_CHANNEL_ACCESS_FAILURE) ||
              (pData->dataCnf.hdr.status == MAC_NO_ACK))
          {
          }
          */
          mac_msg_deallocate((uint8**)&pData->dataCnf.pDataReq);
          break;

        /* Received data INDICATOR */
        case MAC_MCPS_DATA_IND:
          pData = (macCbackEvent_t*)pMsg;
          /* ADD PROCESSING here */
          if (GW_DataCheck ( pData->dataInd.msdu.p, pData->dataInd.msdu.len ))
          {
            HalLedSet (HAL_LED_3, HAL_LED_MODE_TOGGLE);
          }
          break;
      }

      /* Deallocate */
      mac_msg_deallocate((uint8 **)&pMsg);
    }

    return events ^ SYS_EVENT_MSG;
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

  uint8 len = gw_cbackSizeTable[pData->hdr.event];
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

/**************************************************************************************************
 * @brief   Set device as coordinator
 * @param   n/a
 * @return  None
 **************************************************************************************************/
void GW_Startup()
{
  macMlmeStartReq_t   startReq;
  /* Beacon payload, this is used to determine if the device is not zigbee device */
  uint8         gw_BeaconPayload[] = {0x22, 0x33, 0x44};
  uint8         gw_BeaconPayloadLen = 3;

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
  /* Setup MAC_ASSOCIATION_PERMIT */
  MAC_MlmeSetReq(MAC_ASSOCIATION_PERMIT, &gw_MACTrue);
  /* Fill in the information for the start request structure */
  startReq.startTime                = 0;
  startReq.panId                    = gw_PanId;
  startReq.logicalChannel           = NWK_MAC_CHANNEL;
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


/**************************************************************************************************
 * @brief   This routine is called by Associate_Ind inorder to return the response to the device
 * @param   pMsg - pointer to the structure recieved by MAC_MLME_ASSOCIATE_IND
 * @return  None
 **************************************************************************************************/
void GW_AssociateRsp(macCbackEvent_t* pMsg)
{
  /* Assign the short address  for the Device, from pool */
  uint16 assocShortAddress = gw_DevShortAddrList[gw_NumOfDevices];

  /* Build the record for this device */
  gw_DeviceRecord[gw_NumOfDevices].devShortAddr = gw_DevShortAddrList[gw_NumOfDevices];
  gw_DeviceRecord[gw_NumOfDevices].isDirectMsg  = pMsg->associateInd.capabilityInformation & MAC_CAPABLE_RX_ON_IDLE;
  gw_NumOfDevices++;

  /* If the number of devices are more than MAX_DEVICE_NUM, turn off the association permit */
  if (gw_NumOfDevices == GW_MAX_DEVICE_NUM){
    MAC_MlmeSetReq(MAC_ASSOCIATION_PERMIT, &gw_MACFalse);
    HalUARTPrintStr(HAL_UART_PORT_0, "Deny asso\n");
  }
  /* Fill in association respond message */
  sAddrExtCpy(gw_AssociateRsp.deviceAddress, pMsg->associateInd.deviceAddress);
  gw_AssociateRsp.assocShortAddress  = assocShortAddress;
  gw_AssociateRsp.status             = MAC_SUCCESS;
  gw_AssociateRsp.sec.securityLevel  = MAC_SEC_LEVEL_NONE;
  /* Call Associate Response */
  MAC_MlmeAssociateRsp(&gw_AssociateRsp);
  HalUARTPrintStr(HAL_UART_PORT_0, "Allow asso\n");
}


/**************************************************************************************************
 * @brief   This routine calls the Data Request
 * @param   data       - contains the data that would be sent
 *          dataLength - length of the data that will be sent
 * @return  None
 **************************************************************************************************/
void GW_McpsDataReq(uint8* data, uint8 dataLength, bool directMsg, uint16 dstShortAddr)
{
  macMcpsDataReq_t  *pData;
  static uint8      handle = 0;
/*
  if ((pData = MAC_McpsDataAlloc(dataLength, gw_securityLevel, gw_keyIdMode)) != NULL)
  {
    pData->mac.srcAddrMode = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addrMode = SADDR_MODE_SHORT;
    pData->mac.dstAddr.addr.shortAddr = dstShortAddr;
    pData->mac.dstPanId = gw_PanId;
    pData->mac.msduHandle = handle++;
    pData->mac.txOptions = MAC_TXOPTION_ACK;
*/
    /* MAC security parameters */
/*    osal_memcpy( pData->sec.keySource, gw_keySource, MAC_KEY_SOURCE_MAX_LEN );
    pData->sec.securityLevel = gw_securityLevel;
    pData->sec.keyIdMode = gw_keyIdMode;
    pData->sec.keyIndex = gw_keyIndex;
*/
    /* If it's the coordinator and the device is in-direct message */
    //if (!directMsg)
    //{
      //pData->mac.txOptions |= MAC_TXOPTION_INDIRECT;
    //}

    /* Copy data */
  //  osal_memcpy (pData->msdu.p, data, dataLength);

    /* Send out data request */
//    MAC_McpsDataReq(pData);
//  }

}


/**************************************************************************************************
 * @brief   Check if the data match with the predefined data
 * @param    data - pointer to the buffer where the data will be checked against the predefined data
 *           dataLength - length of the data
 * @return  TRUE if the data matched else it's the response / echo packet
 **************************************************************************************************/
bool GW_DataCheck(uint8* data, uint8 dataLength)
{
  uint8 i = 0;

  if (data[0] == dataLength)
  {
    for (i=GW_HEADER_LENGTH; i<(data[0] - 1); i++)
    {
       if (data[i] != gw_Data1[i])
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
void GW_HandleKeys(uint8 keys, uint8 shift)
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
  uartConfig.callBackFunc         = Gateway_UARTCallBack;

  HalUARTOpen (HAL_UART_PORT_0, &uartConfig);
}


///////////////////////////////////////////////////////////
void Gateway_UARTCallBack (uint8 port, uint8 event){
    //do nothing now
}