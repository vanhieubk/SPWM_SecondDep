#ifndef __NWK_COMM_H
#define __NWK_COMM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"

/**************************************************************************************************
 *                                        User's  Defines
 **************************************************************************************************/

#define NWK_MAC_CHANNEL           MAC_CHAN_11   /* Default channel - change it to desired channel */
#define NWK_PAN_ID                0x11CC        /* PAN ID */
#define NWK_GW_SHORT_ADDR         0xAABB        /* Coordinator short address */
#define NWK_DIRECT_MSG_ENABLED    FALSE          /* True if direct messaging is used, False if polling is used */

#define NWK_MAC_BEACON_ORDER      15            /* Setting beacon order to 15 will disable the beacon */
#define NWK_MAC_SUPERFRAME_ORDER  15            /* Setting superframe order to 15 will disable the superframe */
#define NWK_PACKET_LENGTH         102             /* Min = 4, Max = 102 */

#define NWK_MAC_MAX_RESULTS       32             /* Maximun number of scan result that will be accepted */
#define NWK_EBR_PERMITJOINING     TRUE
#define NWK_EBR_LINKQUALITY       1
#define NWK_EBR_PERCENTFILTER     0xFF

#if (NWK_MAC_SUPERFRAME_ORDER > NWK_MAC_BEACON_ORDER)
#error "ERROR! Superframe order cannot be greater than beacon order."
#endif

#if ((NWK_MAC_SUPERFRAME_ORDER != 15) || (NWK_MAC_BEACON_ORDER != 15)) && (NWK_DIRECT_MSG_ENABLED == FALSE)
#error "ERROR! Cannot run beacon enabled on a polling device"
#endif

#if (NWK_PACKET_LENGTH < 4) || (NWK_PACKET_LENGTH > 102)
#error "ERROR! Packet length has to be between 4 and 102"
#endif

/**** VARIABLES  *****/
extern /* Contains pan descriptor results from scan */
macPanDesc_t  panDesc[NWK_MAC_MAX_RESULTS];

/****  FUNCTIONs  ****/
void NWK_ScanReq(uint8 scanType, uint8 scanDuration);


#ifdef __cplusplus
}
#endif

#endif /* __NWK_COM */
