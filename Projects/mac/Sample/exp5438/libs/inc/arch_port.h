#ifndef __ARCH_PORT_H
#define __ARCH_PORT_H

#ifdef __cplusplus
 extern "C" {
#endif

#define _MSP430_CPU
//#define _CORTEX_M3_CPU

#ifdef _MSP430_CPU
  #include "hal_types.h"

  typedef uint32 uint32_t;
  typedef uint16 uint16_t;
  typedef uint8  uint8_t;

  typedef int32  int32_t;
  typedef int16  int16_t;
  typedef int8   int8_t;
#else
  #error "Unsupported CPU";
#endif /* #ifdef _..._CPU */


#ifdef __cplusplus
 }
#endif

#endif /*__ARCH_PORT_H*/

