/*******************************************************************************
* File Name: touch.c
*
*  Description: This file contains the implementation of the user interface
*   using CapSense.
*
******************************************************************************
* Copyright (2020), Cypress Semiconductor Corporation.
******************************************************************************
* This software is owned by Cypress Semiconductor Corporation (Cypress) and is
* protected by and subject to worldwide patent protection (United States and
* foreign), United States copyright laws and international treaty provisions.
* Cypress hereby grants to licensee a personal, non-exclusive, non-transferable
* license to copy, use, modify, create derivative works of, and compile the
* Cypress Source Code and derivative works for the sole purpose of creating
* custom software in support of licensee product to be used only in conjunction
* with a Cypress integrated circuit as specified in the applicable agreement.
* Any reproduction, modification, translation, compilation, or representation of
* this software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, WITH
* REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising out
* of the application or use of any product or circuit described herein. Cypress
* does not authorize its products for use as critical components in life-support
* systems where a malfunction or failure may reasonably be expected to result in
* significant injury to the user. The inclusion of Cypress' product in a life-
* support systems application implies that the manufacturer assumes all risk of
* such use and in doing so indemnifies Cypress against all charges. Use may be
* limited by and subject to the applicable Cypress software license agreement.
*****************************************************************************/

#include "touch.h"
#include "usb_comm.h"

#include "cycfg.h"
#include "cy_sysint.h"

#include "rtos.h"

/*******************************************************************************
* Touch Application Constants
*******************************************************************************/
#define TOUCH_BASELINE_UPDATE           1u
#define TOUCH_BASELINE_IDLE             -1u

/*******************************************************************************
* Local Functions
*******************************************************************************/
static void capsense_isr(void);
static void capsense_eos(cy_stc_active_scan_sns_t* active_scan_sns_ptr);

/*******************************************************************************
* Global Variables
*******************************************************************************/
bool     touch_scan_enable = true;
int32_t  touch_init_baseline = TOUCH_BASELINE_IDLE;

uint8_t  touch_playlist_control_report;
uint8_t  touch_audio_control_status;
uint8_t  touch_prev_report = 0;
uint32_t touch_volume_threshold = 0;

TaskHandle_t     touch_task;
touch_callback_t touch_callback = NULL;
touch_status_t   touch_current_state = {0};
touch_status_t   touch_previous_state = {0};
uint32_t         touch_enable_events = 0;

/*******************************************************************************
* Function Name: touch_init
********************************************************************************
* Summary:
*   Initialize the CapSense Block
*
*******************************************************************************/
void touch_init(void)
{
    /* Get this task handler */
    touch_task = xTaskGetCurrentTaskHandle();

    /* CapSense interrupt configuration */
    const cy_stc_sysint_t CapSense_interrupt_config =
    {
        .intrSrc = CYBSP_CSD_IRQ,
        .intrPriority = CYHAL_ISR_PRIORITY_DEFAULT,
    };

    /* Capture the CSD HW block and initialize it to the default state. */
    Cy_CapSense_Init(&cy_capsense_context);

    /* Initialize CapSense interrupt */
    Cy_SysInt_Init(&CapSense_interrupt_config, capsense_isr);
    NVIC_ClearPendingIRQ(CapSense_interrupt_config.intrSrc);
    NVIC_EnableIRQ(CapSense_interrupt_config.intrSrc);

    /* Register end of scan callback */
    Cy_CapSense_RegisterCallback(CY_CAPSENSE_END_OF_SCAN_E,
                                 capsense_eos, &cy_capsense_context);


    /* Initialize the CapSense firmware modules. */
    Cy_CapSense_Enable(&cy_capsense_context);
}

/*******************************************************************************
* Function Name: touch_is_ready
********************************************************************************
* Summary:
*   Check if ready to process any touches
*
* Return:
*   True is ready, otherwise false.
*
*******************************************************************************/
bool touch_is_ready(void)
{
    return (CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context));
}

/*******************************************************************************
* Function Name: touch_start_scan
********************************************************************************
* Summary:
*   Start scanning for touches.
*
*******************************************************************************/
void touch_start_scan(void)
{
    touch_scan_enable = true;
    xTaskNotify(touch_task, 0, eNoAction);
}

/*******************************************************************************
* Function Name: touch_stop_scan
********************************************************************************
* Summary:
*   Stop scanning for touches.
*
*******************************************************************************/
void touch_stop_scan(void)
{
    touch_scan_enable = false;
}

/*******************************************************************************
* Function Name: touch_get_state
********************************************************************************
* Summary:
*   Return the current state of the sensors (polling method)
*
* Paramters:
*   sensors: structure with sensor state
*
*******************************************************************************/
void touch_get_state(touch_status_t *sensors)
{
    if (sensors != NULL)
    {
        memcpy(sensors, &touch_current_state, sizeof(touch_status_t));
    }
}

/*******************************************************************************
* Function Name: touch_register_callback
********************************************************************************
* Summary:
*   Register a callback to be executed on certain events
*
* Parameters:
*   callback: function pointer to be executed on event
*
*******************************************************************************/
void touch_register_callback(touch_callback_t callback)
{
    touch_callback = callback;
}

/*******************************************************************************
* Function Name: touch_enable_event
********************************************************************************
* Summary:
*   Events to enable on touch.
*
* Parameters:
*   event: Mask of events to enable
*   enable: true to enable, false to disable
*
*******************************************************************************/
void touch_enable_event(touch_event_t event, bool enable)
{
    if (enable)
    {
        touch_enable_events |= event;
    }
    else
    {
        touch_enable_events &= (~event);
    }
}

/*******************************************************************************
* Function Name: touch_update_baseline
********************************************************************************
* Summary:
*   Update the internal baseline of CapSense.
*
*******************************************************************************/
void touch_update_baseline(void)
{
    touch_init_baseline = TOUCH_BASELINE_UPDATE;
}

/*******************************************************************************
* Function Name: touch_process
********************************************************************************
* Summary:
*   Main process for the touch task. Handles the CapSense touches and issues
*   USB commands.
*
*******************************************************************************/
void touch_process(void *arg)
{
    cy_stc_capsense_touch_t* slider_touch_info;

    touch_init();

    while (1)
    {
        /* Check if should keeping scanning */
        if (touch_scan_enable == false)
        {
            /* Wait for start of scan */
            xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
        }

        /* Start a scan */
        Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

        /* Wait till an end of scan */
        xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);

        /* Process all widgets */
        Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);

        /* Update button touch states */
        touch_current_state.button0 = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON0_WDGT_ID,
                                                                 CY_CAPSENSE_BUTTON0_SNS0_ID,
                                                                 &cy_capsense_context);
        touch_current_state.button1 = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON1_WDGT_ID,
                                                                 CY_CAPSENSE_BUTTON1_SNS0_ID,
                                                                 &cy_capsense_context);

        /* Get slider status */
        slider_touch_info = Cy_CapSense_GetTouchInfo(
               CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, &cy_capsense_context);
        touch_current_state.slider_status = (slider_touch_info->numPosition > 0);
        touch_current_state.slider_pos = slider_touch_info->ptrPosition->x;

        /* Check if a callback was registered */
        if (touch_callback != NULL)
        {
            /* Check if should handle TOUCH LIFT events */
            if (touch_enable_events & TOUCH_LIFT)
            {
                if (touch_previous_state.button0 && (!touch_current_state.button0))
                {
                    touch_callback(CY_CAPSENSE_BUTTON0_WDGT_ID, TOUCH_LIFT, 0);
                }

                if (touch_previous_state.button1 && (!touch_current_state.button1))
                {
                    touch_callback(CY_CAPSENSE_BUTTON1_WDGT_ID, TOUCH_LIFT, 0);
                }

                if (touch_previous_state.slider_status && (!touch_current_state.slider_status))
                {
                    touch_callback(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, TOUCH_LIFT, touch_current_state.slider_pos);
                }
            }

            /* Check if should handle TOUCH DOWN events */
            if (touch_enable_events & TOUCH_DOWN)
            {
                if ((!touch_previous_state.button0) && touch_current_state.button0)
                {
                    touch_callback(CY_CAPSENSE_BUTTON0_WDGT_ID, TOUCH_DOWN, 0);
                }

                if ((!touch_previous_state.button1) && touch_current_state.button1)
                {
                    touch_callback(CY_CAPSENSE_BUTTON1_WDGT_ID, TOUCH_DOWN, 0);
                }

                if ((!touch_previous_state.slider_status) && touch_current_state.slider_status)
                {
                    touch_callback(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, TOUCH_DOWN, touch_current_state.slider_pos);
                }
            }

            /* Check if should handle TOUCH SLIDE RIGHT events */
            if (touch_enable_events & TOUCH_SLIDE_RIGHT)
            {
                if (touch_previous_state.slider_pos < touch_current_state.slider_pos)
                {
                    touch_callback(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, TOUCH_SLIDE_RIGHT, touch_current_state.slider_pos);
                }
            }

            /* Check if should handle TOUCH SLIDE LEFT events */
            if (touch_enable_events & TOUCH_SLIDE_LEFT)
            {
                if (touch_previous_state.slider_pos > touch_current_state.slider_pos)
                {
                    touch_callback(CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, TOUCH_SLIDE_LEFT, touch_current_state.slider_pos);
                }
            }
        }

        /* Update the previous state */
        touch_get_state(&touch_previous_state);

        /* Check if baseline should be updated */
        if (touch_init_baseline == 0)
        {
            Cy_CapSense_InitializeAllBaselines(&cy_capsense_context);
            touch_init_baseline = TOUCH_BASELINE_IDLE;
        }
        else if (touch_init_baseline > 0)
        {
            touch_init_baseline--;
        }

        /* Wait for the touch period */
        vTaskDelay(pdMS_TO_TICKS(TOUCH_PERIOD_MS));
    }
}

/*******************************************************************************
* Function Name: capsense_isr
********************************************************************************
* Summary:
*  Wrapper function for handling interrupts from CapSense block.
*
*******************************************************************************/
static void capsense_isr(void)
{
    Cy_CapSense_InterruptHandler(CYBSP_CSD_HW, &cy_capsense_context);
}

/*******************************************************************************
* Function Name: capsense_eos
********************************************************************************
* Summary:
*  CapSense end of scan callback function. This function sends a command to
*  CapSense task to process scan.
*
* Parameters:
*  cy_stc_active_scan_sns_t * active_scan_sns_ptr (unused)
*
*******************************************************************************/
void capsense_eos(cy_stc_active_scan_sns_t* active_scan_sns_ptr)
{
    BaseType_t xYieldRequired;

    (void)active_scan_sns_ptr;

    xTaskNotifyFromISR(touch_task, 0, eNoAction, &xYieldRequired);

    portYIELD_FROM_ISR(xYieldRequired);
}


/* [] END OF FILE */

