/* OS includes */
#include "OSAL.h"
/* MAC Application Interface */
#include "mac_api.h"

#include "nwk_comm.h"


/**** VARIABLEs  ****/
macPanDesc_t  panDesc[NWK_MAC_MAX_RESULTS];



/**************************************************************************************************
 * @brief   Performs active scan on specified channel
 * @param   None
 * @return  None
 **************************************************************************************************/
void NWK_ScanReq(uint8 scanType, uint8 scanDuration)
{
  macMlmeScanReq_t scanReq;

  /* Fill in information for scan request structure */
  scanReq.scanChannels  = (uint32) 1 << NWK_MAC_CHANNEL;
  scanReq.scanType      = scanType;
  scanReq.scanDuration  = scanDuration;
  scanReq.maxResults    = NWK_MAC_MAX_RESULTS;
  scanReq.permitJoining = NWK_EBR_PERMITJOINING;
  scanReq.linkQuality   = NWK_EBR_LINKQUALITY;
  scanReq.percentFilter = NWK_EBR_PERCENTFILTER;
  scanReq.result.pPanDescriptor = panDesc;
  osal_memset(&scanReq.sec, 0, sizeof(macSec_t));
  /* Call scan request */
  MAC_MlmeScanReq(&scanReq);
}
