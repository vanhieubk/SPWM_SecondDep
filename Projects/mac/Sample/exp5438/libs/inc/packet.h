#ifndef __PACKET_H
#define __PACKET_H

#include "sensing.h"
/* define packet type */
#define PKT_SENSING_TYPE        1
#define PKT_ALIVE_TYPE          2
typedef uint8      pktType_t;

typedef struct{
  uint8     nodeId;
  uint32    curTime;
} alivePara_t;

typedef struct{
  uint8     nodeId;
  uint32    time;
  uint16    run;
  uint16    storeBlock;
  sensing_t sensingData;
} sensingPara_t;

typedef struct{
  pktType_t  pktType;
  union{
    alivePara_t    alivePara;
    sensingPara_t  sensingPara;
  } pktPara;
} pkt_t;

void PKT_Print(pkt_t* pkt);

#endif /* __PACKET_H */
