/* OS includes */
#include "OSAL.h"
/* MAC Application Interface */
#include "mac_api.h"

#include "nwk_comm.h"


/**** VARIABLEs  ****/
scan_result_t scanResult;


/**************************************************************************************************
 * @brief   Performs active scan on specified channel
 * @param   None
 * @return  None
 **************************************************************************************************/
void NWK_ScanReq(uint8 scanType, uint8 scanDuration)
{
  macMlmeScanReq_t scanReq;

  /* Fill in information for scan request structure */
  scanReq.scanChannels  = NWK_SCAN_CHANNELS;
  scanReq.scanType      = scanType;
  scanReq.scanDuration  = scanDuration;
  scanReq.maxResults    = NWK_MAC_MAX_RESULTS;
  scanReq.permitJoining = NWK_EBR_PERMITJOINING;
  scanReq.linkQuality   = NWK_EBR_LINKQUALITY;
  scanReq.percentFilter = NWK_EBR_PERCENTFILTER;
  if (scanType != MAC_SCAN_ED){
    scanReq.result.pPanDescriptor = (macPanDesc_t*) &scanResult;
  }
  else{
    scanReq.result.pEnergyDetect = (uint8*) &scanResult;
  }
  osal_memset(&scanReq.sec, 0, sizeof(macSec_t));
  /* Call scan request */
  MAC_MlmeScanReq(&scanReq);
}
