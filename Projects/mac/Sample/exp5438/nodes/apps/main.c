/* Hal Drivers */
#include "hal_types.h"
#include "hal_key.h"
#include "hal_timer.h"
#include "hal_drivers.h"
#include "hal_led.h"

#ifdef HAL_BOARD_CC2538
  #include "hal_systick.h"
#endif

/* MAC Application Interface */
#include "mac_api.h"
/* Application */
#include "node.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OnBoard.h"
#include "OSAL_PwrMgr.h"

/**************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/
/* This callback is triggered when a key is pressed */
void NODE_KeyCallback(uint8 keys, uint8 state);

/* This function invokes osal_start_system */
void nodeOSTask(void *task_parameter);

#define OSAL_START_SYSTEM() st(                                         \
  osal_start_system();                                                  \
)

/**************************************************************************************************
 * @brief       Start of application.
 * @param       none
 * @return      none
 **************************************************************************************************
 */
int main(void)
{
  /* Initialize hardware */
  HAL_BOARD_INIT();

  /* Initialze the HAL driver */
  HalDriverInit();

  /* Initialize MAC */
  MAC_Init();

  /* Initialize the operating system */
  osal_init_system();

#ifdef HAL_BOARD_CC2538
  /* Master enable interrupts */
  IntMasterEnable();

  /* Setup SysTick to generate interrupt every 1 ms */
  SysTickSetup();
#endif /* HAL_BOARD_CC2538 */

  /* Enable interrupts */
  HAL_ENABLE_INTERRUPTS();

  /* Setup Keyboard callback */
  HalKeyConfig(NODE_KEY_INT_ENABLED, NODE_KeyCallback);
  HalLedBlink(HAL_LED_1, 0, 50, 1000);
  HalLedBlink(HAL_LED_2, 0, 50, 1000);
  HalLedBlink(HAL_LED_3, 0, 50, 1000);
  HalLedBlink(HAL_LED_4, 0, 50, 1000);
  HalLedBlink(HAL_LED_5, 0, 50, 1000);
  /* Start OSAL */
  OSAL_START_SYSTEM();

  return 0;
}

/*********************************************************************
 * @brief   This function invokes osal_start_system(The main event loop)
 * @param   task_parameter(Not being used in the present fucntion)
 * @return  none
 */
void nodeOSTask(void *task_parameter)
{
  osal_start_system();
}

/**************************************************************************************************
                                           CALL-BACKS
**************************************************************************************************/


/**************************************************************************************************
 * @brief   Callback service for keys
 * @param   keys  - keys that were pressed
 *          state - shifted
 * @return  void
 **************************************************************************************************/
void NODE_KeyCallback(uint8 keys, uint8 state)
{
  if ( NODE_TaskId != TASK_NO_TASK )
  {
    NODE_HandleKeys (keys, state);
  }
}

/**************************************************************************************************
 * @brief   Enable/Disable and setup power saving related stuff
 * @param   mode - PWRMGR_ALWAYS_ON or PWRMGR_BATTERY
 * @return  void
 **************************************************************************************************/
void NODE_PowerMgr(uint8 enable)
{
  /* enable OSAL power management */
  if (enable)
   osal_pwrmgr_device(PWRMGR_BATTERY);
  else
   osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
}
