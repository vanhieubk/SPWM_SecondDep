#ifndef __NWK_COMM_H
#define __NWK_COMM_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "hal_types.h"

/***************************/
#define NWK_SCAN_CHANNELS         ( MAC_CHAN_11_MASK | MAC_CHAN_12_MASK | MAC_CHAN_13_MASK | MAC_CHAN_14_MASK \
                                  | MAC_CHAN_15_MASK | MAC_CHAN_16_MASK | MAC_CHAN_17_MASK | MAC_CHAN_18_MASK \
                                  | MAC_CHAN_19_MASK | MAC_CHAN_20_MASK | MAC_CHAN_21_MASK | MAC_CHAN_22_MASK \
                                  | MAC_CHAN_23_MASK | MAC_CHAN_24_MASK | MAC_CHAN_25_MASK | MAC_CHAN_26_MASK )
#define NWK_PAN_ID                0x11CC        /* PAN ID */
#define NWK_GW_SHORT_ADDR         0xAABB        /* Coordinator short address */
#define NWK_DIRECT_MSG_ENABLED    FALSE         /* True if direct messaging is used, False if polling is used */

#define NWK_MAC_BEACON_ORDER      15            /* Setting beacon order to 15 will disable the beacon */
#define NWK_MAC_SUPERFRAME_ORDER  15            /* Setting superframe order to 15 will disable the superframe */
#define NWK_PACKET_LENGTH         100           /* Min = 4, Max = 102 */

#define NWK_MAC_MAX_RESULTS       18            /* Maximun number of scan result that will be accepted */
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

typedef union{ uint8         energy[NWK_MAC_MAX_RESULTS];
               macPanDesc_t  panDesc[NWK_MAC_MAX_RESULTS];
             } scan_result_t;
/**** VARIABLES  *****/
extern  scan_result_t scanResult;
/****  FUNCTIONs  ****/
void NWK_ScanReq(uint8 scanType, uint8 scanDuration);


#ifdef __cplusplus
}
#endif

#endif /* __NWK_COM */
