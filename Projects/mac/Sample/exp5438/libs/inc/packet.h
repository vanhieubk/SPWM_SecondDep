#ifndef __PACKET_H
#define __PACKET_H

/* define packet type */
#define PKT_SENSING_TYPE        1
#define PKT_ALIVE_TYPE          2
typedef uint8      pktType_t;
typedef uint32     alivePara_t;
typedef sensing_t  sensingPara_t;

typedef struct{
  pktType_t  pktType;
  union{
    alivePara_t    alivePara;
    sensingPara_t  sensingPara;
  } pktPara;
} pkt_t;

#endif /* __PACKET_H */
