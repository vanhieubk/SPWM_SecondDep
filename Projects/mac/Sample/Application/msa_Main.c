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
#include "msa.h"

/* OSAL */
#include "OSAL.h"
#include "OSAL_Tasks.h"
#include "OnBoard.h"
#include "OSAL_PwrMgr.h"

/**************************************************************************************************
 * FUNCTIONS
 **************************************************************************************************/
/* This callback is triggered when a key is pressed */
void MSA_Main_KeyCallback(uint8 keys, uint8 state);

/* This function invokes osal_start_system */
void msaOSTask(void *task_parameter);

#ifdef IAR_ARMCM3_LM
#include "osal_task.h"
#define OSAL_START_SYSTEM() st(                                         \
  static uint8 param_to_pass;                                           \
  void * task_handle;                                                   \
  uint32 osal_task_priority;                                            \
                                                                        \
  /* create OSAL task */                                                \
  osal_task_priority = OSAL_TASK_PRIORITY_ONE;                          \
  osal_task_create(msaOSTask, "osal_task", OSAL_MIN_STACK_SIZE,         \
                   &param_to_pass, osal_task_priority, &task_handle);   \
  osal_task_start_scheduler();                                          \
)
#else
#define OSAL_START_SYSTEM() st(                                         \
  osal_start_system();                                                  \
)
#endif /* IAR_ARMCM3_LM */

/**************************************************************************************************
 * @fn          main
 *
 * @brief       Start of application.
 *
 * @param       none
 *
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
  HalKeyConfig(MSA_KEY_INT_ENABLED, MSA_Main_KeyCallback);

  /* Blink LED on startup */
  HalLedBlink (HAL_LED_4, 0, 40, 200);

  /* Start OSAL */
  OSAL_START_SYSTEM();

  return 0;
}

/*********************************************************************
 * @fn      msaOSTask
 *
 * @brief   This function invokes osal_start_system(The main event loop)
 *
 * @param   task_parameter(Not being used in the present fucntion)
 *
 * @return  none
 */
void msaOSTask(void *task_parameter)
{
  osal_start_system();
}

/**************************************************************************************************
                                           CALL-BACKS
**************************************************************************************************/


/**************************************************************************************************
 * @fn      MSA_KeyCallback
 *
 * @brief   Callback service for keys
 *
 * @param   keys  - keys that were pressed
 *          state - shifted
 *
 * @return  void
 **************************************************************************************************/
void MSA_Main_KeyCallback(uint8 keys, uint8 state)
{
  if ( MSA_TaskId != TASK_NO_TASK )
  {
    MSA_HandleKeys (keys, state);
  }
}

/**************************************************************************************************
 *
 * @fn      MSA_PowerMgr
 *
 * @brief   Enable/Disable and setup power saving related stuff
 *
 * @param   mode - PWRMGR_ALWAYS_ON or PWRMGR_BATTERY
 *
 * @return  void
 *
 **************************************************************************************************/
void MSA_PowerMgr(uint8 enable)
{
  /* enable OSAL power management */
  if (enable)
   osal_pwrmgr_device(PWRMGR_BATTERY);
  else
   osal_pwrmgr_device(PWRMGR_ALWAYS_ON);
}

/*************************************************************************************************
**************************************************************************************************/
