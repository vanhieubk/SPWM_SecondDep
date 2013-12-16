#ifndef __STORE_INFO
#define __STORE_INFO

typedef struct{
  uint16_t curBlock; 
  uint16_t nodeId;
  uint16_t numReset;
} node_store_info_t;

typedef struct{
  uint16_t curBlock;
  uint16_t numReset;
} gateway_store_info_t;
  
#endif /* __STORE_INFO */
