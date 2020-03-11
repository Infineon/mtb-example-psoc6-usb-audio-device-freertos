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
#include "cycfg_capsense.h"
#include "cy_sysint.h"

#include "rtos.h"

/*******************************************************************************
* Touch Application Constants
*******************************************************************************/
#define TOUCH_VOL_CHANGE_UPDATE_FREQ    10u
#define TOUCH_SLIDER_NO_TOUCH           0xFFu
#define TOUCH_SLIDER_MIDDLE_POINT       (cy_capsense_context.ptrWdConfig[CY_CAPSENSE_LINEARSLIDER0_WDGT_ID].xResolution/2)

#define TOUCH_RELEASE_BUTTON_0          0x01u
#define TOUCH_RELEASE_BUTTON_1          0x08u
#define TOUCH_SLIDER_LEFT               0x40u
#define TOUCH_SLIDER_RIGHT              0x20u

#define TOUCH_USB_HID_ENDPOINT          0x4u
#define TOUCH_USB_REPORT_SIZE           1u

#define TOUCH_BASELINE_UPDATE           1u
#define TOUCH_BASELINE_IDLE             -1u

#define TOUCH_PERIOD_MS                 10u

/*******************************************************************************
* Local Functions
*******************************************************************************/
static void capsense_isr(void);

/*******************************************************************************
* Global Variables
*******************************************************************************/
bool    touch_button0;
bool    touch_button1;
bool    touch_prev_button0 = false;
bool    touch_prev_button1 = false;
int32_t  touch_init_baseline = TOUCH_BASELINE_IDLE;

uint8_t  touch_playlist_control_report;
uint8_t  touch_audio_control_status;
uint8_t  touch_prev_report = 0;
uint32_t touch_volume_threshold = 0;

/*******************************************************************************
* Function Name: touch_init
********************************************************************************
* Summary:
*   Initialize the CapSense Block
*
*******************************************************************************/
void touch_init(void)
{
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
    uint8_t  slider_touch_status;
    uint16_t slider_pos;

    touch_init();

    /* Start first scan */
    Cy_CapSense_ScanAllWidgets(&cy_capsense_context);

    while (1)
    {
        if((CY_CAPSENSE_NOT_BUSY == Cy_CapSense_IsBusy(&cy_capsense_context)) && usb_comm_is_ready())
        {
            /* Process all widgets */
            Cy_CapSense_ProcessAllWidgets(&cy_capsense_context);

            /* Update button touch states */
            touch_button0 = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON0_WDGT_ID,
                                                       CY_CAPSENSE_BUTTON0_SNS0_ID,
                                                       &cy_capsense_context);
            touch_button1 = Cy_CapSense_IsSensorActive(CY_CAPSENSE_BUTTON1_WDGT_ID,
                                                       CY_CAPSENSE_BUTTON1_SNS0_ID,
                                                       &cy_capsense_context);

            /* Get slider status */
            slider_touch_info = Cy_CapSense_GetTouchInfo(
                    CY_CAPSENSE_LINEARSLIDER0_WDGT_ID, &cy_capsense_context);
            slider_touch_status = slider_touch_info->numPosition;
            slider_pos = slider_touch_info->ptrPosition->x;

            /* Detect the release of button 0 */
            if (touch_prev_button0 && (!touch_button0))
            {
                touch_audio_control_status |= TOUCH_RELEASE_BUTTON_0;
            }

            /* Detect the release of button 1 */
            if (touch_prev_button1 && (!touch_button1))
            {
                touch_audio_control_status |= TOUCH_RELEASE_BUTTON_1;
            }

            /* Detect if the slider was touched */
            if (slider_touch_status != 0)
            {
                /* Check if the control status should be updated */
                if (touch_volume_threshold == 0)
                {
                    if (slider_pos < TOUCH_SLIDER_MIDDLE_POINT)
                    {
                        touch_audio_control_status |= TOUCH_SLIDER_LEFT;
                    }
                    else
                    {
                        touch_audio_control_status |= TOUCH_SLIDER_RIGHT;
                    }
                }

                /* Increment volume update threshold */
                touch_volume_threshold++;

                /* Check if the volume update threshold should be reset */
                if (touch_volume_threshold >= TOUCH_VOL_CHANGE_UPDATE_FREQ)
                {
                    touch_volume_threshold = 0;
                }
            }

            /* Check if a new report should be reported to the USB host */
            if ((touch_audio_control_status != 0) || (touch_prev_report != touch_audio_control_status))
            {
                cy_en_usb_dev_ep_state_t epState;

                epState = Cy_USBFS_Dev_Drv_GetEndpointState(CYBSP_USBDEV_HW, TOUCH_USB_HID_ENDPOINT, &usb_drvContext);

                /* Check that endpoint is ready for operation */
                if ((CY_USB_DEV_EP_IDLE == epState) || (CY_USB_DEV_EP_COMPLETED == epState))
                {
                    touch_playlist_control_report = touch_audio_control_status;

                    /* Send out report data */
                    Cy_USB_Dev_WriteEpNonBlocking(TOUCH_USB_HID_ENDPOINT,
                          &touch_playlist_control_report,
                          TOUCH_USB_REPORT_SIZE,
                          &usb_devContext);

                    touch_prev_report = touch_audio_control_status;

                    touch_audio_control_status = 0;
                }
            }

            /* Update the previous button variables */
            touch_prev_button0 = touch_button0;
            touch_prev_button1 = touch_button1;

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

            /* Start next scan */
            Cy_CapSense_ScanAllWidgets(&cy_capsense_context);
        }

        /* Run this task every 10 ms */
        vTaskDelay(TOUCH_PERIOD_MS/portTICK_PERIOD_MS);
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


/* [] END OF FILE */

