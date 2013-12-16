/**
 * @file glcd.c
 * @author Huynh Tan Manh <tanmanh0707@gmail.com>
 * @version 1.0
 * @date 01-SEP-2013
 *
 * @copyright
 * This project and all its relevant hardware designs, documents, source codes, compiled libraries
 * belong to <b> Smart Sensing and Intelligent Controlling Group (SSAIC Group) </b>\n
 * You are prohibited to broadcast, distribute, copy, modify, print, or reproduce this in anyway
 * without written permission from SSAIC\n
 * <b> Copyright (C) 2013 by SSAIC, All Right Reversed </b>
 * @attention
 * @brief
 * Implementing graphic LCD functions for 128x64 Graphic LCD
 */ 

#include "glcd.h"

//////////////////////////////////////////////////////
//	DEFINE	/////////
#if ( defined(GLCD_128X64_KS107) || defined(GLCD_128X64_HD61202) || defined(GLCD_128X64_ST7920) )
  #define GLCD_WIDTH			128
  #define GLCD_HEIGHT			64
#else
  #error "Unsupported Graphic LCD"
#endif
//////////////////////////////////////////////////////
//	PINs configuration and MACROs	/////////
/* these MACROs will be implemented by lecturers */
#if ( defined(GLCD_128X64_KS107) || defined(GLCD_128X64_HD61202) )
  #ifdef GLCD_MSP430F5xxx_MCU
    /* redefine PINs configuration corresponding with your board */
	#define GLCD_RS_PORT		P7OUT
	#define GLCD_RS_DDR			P7DIR
	#define GLCD_RS_PIN			BV(4)

	#define GLCD_RNW_PORT		/* do not use in current platform */
	#define GLCD_RNW_DDR        /* do not use in current platform */
	#define GLCD_RNW_PIN        /* do not use in current platform */
	#define GLCD_EN_PORT        P5OUT 
	#define GLCD_EN_DDR			P5DIR
	#define GLCD_EN_PIN			BV(1)
	#define GLCD_CS1_PORT		P4OUT
	#define GLCD_CS1_DDR		P4DIR
	#define GLCD_CS1_PIN        BV(7)
	#define GLCD_CS2_PORT       P5OUT
	#define GLCD_CS2_DDR		P5DIR
	#define GLCD_CS2_PIN        BV(0)
	#define GLCD_NRST_PORT      P4OUT
	#define GLCD_NRST_DDR		P4DIR
	#define GLCD_NRST_PIN		BV(3)
	#define GLCD_LED_ON_PORT	P4OUT
	#define GLCD_LED_ON_DDR		P4DIR
	#define GLCD_LED_ON_PIN		BV(2)
	#define GLCD_DATA_PORT		P6OUT
	#define GLCD_DATA_DDR		P6DIR
	#define GLCD_DATA_SIZE      8
	#define GLCD_DATA_SHIFT     0
	/* primitive output changing MACROs (do not depend on your board)*/
	#define GLCD_PIN_MODE_OUT(port, pin) (port) |=  (pin)
	#define GLCD_SET_PIN(port, pin)  	 (port) |=  (pin)
    #define GLCD_CLR_PIN(port, pin) 	 (port) &= ~(pin)
	
   
  #elif (defined (GLCD_STM32F10x_MCU))
  #elif (defined (GLCD_STM32F40x_MCU))
  #else
    #error "Unsupported MCU"
  #endif /* GLCD_..._MCU */	
#else
  #error "Unsupported Graphic LCD"
#endif /* GLCD_128X64_... */


//////////////////////////////////////////////////////
//	PRIVATE FUNCTION DECLARATION	/////////
void _GLCD_LowLevelInit(void);
void _GLCD_WriteCmd(uint8_t cmd, uint8_t chip);
void _GLCD_WriteData(uint8_t data, uint8_t chip);
void _GLCD_DelayUs(uint16_t delayTime);

//////////////////////////////////////////////////////
//	PRIVATE FUNCTION IMPLEMENTATION	/////////
/* these functions will be implemented by lecturers */
#if ( defined(GLCD_128X64_KS107) || defined(GLCD_128X64_HD61202) )
  #ifdef GLCD_MSP430F5xxx_MCU
	void _GLCD_LowLevelInit(void){
	  GLCD_PIN_MODE_OUT(GLCD_RS_DDR,  	 GLCD_RS_PIN);
	  /* do not use in this platform */
	  //GLCD_PIN_MODE_OUT(GLCD_RNW_DDR, 	 GLCD_RNW_PIN);
	  //GLCD_CLR_PIN(GLCD_RNW_PORT, GLCD_RNW_PIN);
	  GLCD_PIN_MODE_OUT(GLCD_EN_DDR,  	 GLCD_EN_PIN);
	  GLCD_CLR_PIN(GLCD_EN_PORT, GLCD_EN_PIN);
	  GLCD_PIN_MODE_OUT(GLCD_CS1_DDR, 	 GLCD_CS1_PIN);
	  GLCD_PIN_MODE_OUT(GLCD_CS2_DDR, 	 GLCD_CS2_PIN);
	  /* set or clear chip select BELOW*/
	  GLCD_PIN_MODE_OUT(GLCD_NRST_DDR,   GLCD_NRST_PIN);
	  GLCD_CLR_PIN(GLCD_NRST_PORT, GLCD_NRST_PIN);
	  GLCD_PIN_MODE_OUT(GLCD_LED_ON_DDR, GLCD_LED_ON_PIN);
	  GLCD_SET_PIN(GLCD_LED_ON_PORT, GLCD_LED_ON_PIN);
	  #if (GLCD_DATA_SIZE == 8)
		GLCD_DATA_DDR |= 0xFF;
	  #else
		#error "Unsupported LCD data size"
	  #endif
	  //_GLCD_DelayUs(???);
	  GLCD_SET_PIN(GLCD_NRST_PORT, GLCD_NRST_PIN);
	  //_GLCD_DelayUs(???);
	}
	
	
	void _GLCD_WriteCmd(uint8_t cmd, uint8_t chip){
	  if (1 == chip){ 
	    GLCD_SET_PIN(GLCD_CS1_PORT, GLCD_CS1_PIN);
	  }
      else{
        GLCD_SET_PIN(GLCD_CS2_PORT, GLCD_CS2_PIN);
      }
	  GLCD_CLR_PIN(GLCD_RS_PORT, GLCD_RS_PIN);
	  /* GLCD_CLR_PIN(GLCD_RNW_PORT, GLCD_RNW_PIN); */
      #if (GLCD_DATA_SIZE == 8)
		GLCD_DATA_PORT = cmd;
		GLCD_SET_PIN(GLCD_EN_PORT, GLCD_EN_PIN);
		_GLCD_DelayUs(2);
		GLCD_CLR_PIN(GLCD_EN_PORT, GLCD_EN_PIN);
		GLCD_CLR_PIN(GLCD_CS1_PORT, GLCD_CS1_PIN);
		GLCD_CLR_PIN(GLCD_CS2_PORT, GLCD_CS2_PIN);
	  #else
		#error "Unsupported LCD data size"
	  #endif	  
	}
	
	
	void _GLCD_WriteData(uint8_t data, uint8_t chip){
	  if (1 == chip){ 
	    GLCD_SET_PIN(GLCD_CS1_PORT, GLCD_CS1_PIN);
	  }
      else{
        GLCD_SET_PIN(GLCD_CS2_PORT, GLCD_CS2_PIN);
      }
	  GLCD_SET_PIN(GLCD_RS_PORT, GLCD_RS_PIN);
	  /* GLCD_CLR_PIN(GLCD_RNW_PORT, GLCD_RNW_PIN); */
      #if (GLCD_DATA_SIZE == 8)
		GLCD_DATA_PORT = data;
		GLCD_SET_PIN(GLCD_EN_PORT, GLCD_EN_PIN);
		_GLCD_DelayUs(2);
		GLCD_CLR_PIN(GLCD_EN_PORT, GLCD_EN_PIN);
		GLCD_CLR_PIN(GLCD_CS1_PORT, GLCD_CS1_PIN);
		GLCD_CLR_PIN(GLCD_CS2_PORT, GLCD_CS2_PIN);
	  #else
		#error "Unsupported LCD data size"
	  #endif	
	}
	
	
	void _GLCD_DelayUs(uint16_t delayTime){
	   uint16_t count, time;
	   for (time=0; time<delayTime; time++){
	     for (count=0; count<4; count++);
	   }
	}

  #else
    #error "Unsupported MCU"
  #endif /* GLCD_..._MCU */	
#else
  #error "Unsupported Graphic LCD"
#endif /* GLCD_128X64_... */
//////////////////////////////////////////////////////
//	PUBLIC FUNCTION IMPLEMENTATION	/////////
/* Initial and Setting Functions */
/**
 * @brief Init LCD, set cursor to home
 * @return none
 */
void GLCD_Init(void){
  _GLCD_LowLevelInit();
  /* student adds code here */
  /* _GLCD_WriteCmd(...);
     _GLCD_DelayMs(...);
	 _GLCD_WriteCmd(...);
     _GLCD_DelayMs(...);
	 ...
  */ 
}


//void GLCD_SetFontSize(uint8_t size){}
//void GLCD_SetCursor(uint8_t mode, uint16_t row, uint16_t col, uint8_t type){}
//void GLCD_SetAttrb(uint16_t row, uint16_t col, uint8_t mode){}

/**
 * @brief Clear text layer on LCD
 * @return none
 */
void GLCD_ClrTxt(void){
  /* student adds code here */
}


/**
 * @brief Clear graphic layer on LCD
 * @return none
 */
void GLCD_ClrGrh(void){
  /* student adds code here */
}


/**
 * @brief Set text cursor to new location
 * @param row 
 * @param col
 * @return none
 */
void GLCD_Goto(uint16_t row, uint16_t col){
  /* student adds code here */
}


/**
 * @brief Print one character at current location
 * @param outChar
 * @return none
 */
void GLCD_PrintChar(char outChar){
  /* student adds code here */
}


/**
 * @brief Print one character at specified location
 * @param outChar
 * @param row
 * @param col
 * @return none
 */
void GLCD_PrintCharAt(char outChar, uint16_t row, uint16_t col){
  GLCD_Goto(row, col);
  /* student adds code here */
}


/**
 * @brief Print one unsigned integer at current location in decimal
 * @param outNum
 * @return none
 */
void GLCD_PrintUInt(uint32_t outNum){
  /* student adds code here */
}


/**
 * @brief Print one unsigned integer at specified location in decimal
 * @param outNum
 * @param row
 * @param col
 * @return none
 */
void GLCD_PrintUIntAt(uint32_t outNum, uint16_t row, uint16_t col){
  GLCD_Goto(row, col);
  /* student adds code here */
}


/**
 * @brief Print one signed integer at current location in decimal
 * @param outNum
 * @return none
 */
void GLCD_PrintInt(int32_t outNum){
}


/**
 * @brief Print one signed integer at specified location in decimal
 * @param outNum
 * @param row
 * @param col
 * @return none
 */
void GLCD_PrintIntAt(int32_t outNum, uint16_t row, uint16_t col){
  GLCD_Goto(row, col);
  /* student adds code here */
}


/**
 * @brief Print one string at current location
 * @param outStr
 * @return none
 */
void GLCD_PrintStr(char *outStr){
}


/**
 * @brief Print one string at specified location
 * @param outStr
 * @param row
 * @param col
 * @return none
 */
void GLCD_PrintStrAt(char *outStr, uint16_t row, uint16_t col){
  GLCD_Goto(row, col);
  /* student adds code here */
}


/**
 * @brief Set one pixel at specified location
 * @param row
 * @param col
 * @param onOff <b>1 </b>set the pixel, <b> 0 </b> clear the pixel
 * @return none
 */
void GLCD_SetPixel(uint16_t row, uint16_t col, uint8_t onOff){
  GLCD_Goto(row, col);
  /* student adds code here */
}


//void GLCD_PixFontAt(uint16_t row, uint16_t col, uint8_t* textptr, uint8_t size, uint8_t color){}
/**
 * @brief Draw a vertical line 
 * @param row
 * @param lineWidth
 * @param onOff <b>1 </b>set the pixel, <b> 0 </b> clear the pixel
 * @return none
 */
void GLCD_DrawVerticalLine(uint16_t row, uint8_t lineWidth, uint8_t onOff){
  GLCD_Goto(row, 0);
  /* student adds code here
  for (i=0; i< */
}


/**
 * @brief Draw a horizontal line 
 * @param col
 * @param lineWidth
 * @param onOff <b>1 </b>set the pixel, <b> 0 </b> clear the pixel
 * @return none
 */
void GLCD_DrawHorizontalLine(uint16_t col, uint8_t lineWidth, uint8_t onOff){
  GLCD_Goto(0, col);
  /* student adds code here
  for (i=0; i< */ 
}
