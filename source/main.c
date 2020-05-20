/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the USB Audio Device Example
*              for ModusToolbox.
*
* Related Document: See Readme.md
*
*
*******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*******************************************************************************/

#include "cy_pdl.h"
#include "cyhal.h"
#include "cybsp.h"
#include "cycfg.h"

#include "audio_app.h"
#include "audio_out.h"
#include "audio_in.h"
#include "touch.h"

#include "rtos.h"

/*******************************************************************************
* Global Variables
********************************************************************************/
/* RTOS tasks */
TaskHandle_t rtos_audio_app_task;
TaskHandle_t rtos_audio_in_task;
TaskHandle_t rtos_audio_out_task;
TaskHandle_t rtos_touch_task;

/* RTOS Event Group */
EventGroupHandle_t rtos_events;

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  Main function of Cortex-M4. Creates all RTOS related elements and runs the
*  RTOS scheduler.
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Enable global interrupts */
    __enable_irq();

    /* Create the RTOS tasks */
    xTaskCreate(audio_app_process, "Audio App Task",
                RTOS_STACK_DEPTH, NULL, RTOS_TASK_PRIORITY,
                &rtos_audio_app_task);

    xTaskCreate(audio_in_process, "Audio In Task",
                RTOS_STACK_DEPTH, NULL, RTOS_TASK_PRIORITY,
                &rtos_audio_in_task);

    xTaskCreate(audio_out_process, "Audio Out Task",
                RTOS_STACK_DEPTH, NULL, RTOS_TASK_PRIORITY,
                &rtos_audio_out_task);

    xTaskCreate(touch_process, "Touch Task",
                RTOS_STACK_DEPTH, NULL, RTOS_TASK_PRIORITY,
                &rtos_touch_task);

    /* Create RTOS Event Group */
    rtos_events = xEventGroupCreate();

    /* Start the RTOS Scheduler */
    vTaskStartScheduler();

    /* Should never get there */

    return 0;
}

/*******************************************************************************
* Function Name: vApplicationIdleHook()
********************************************************************************
* Summary:
*   RTOS Idle task implementation.
*
*******************************************************************************/
void vApplicationIdleHook( void )
{
    /* Go to sleep */
    cyhal_system_sleep();
}



/* [] END OF FILE */
