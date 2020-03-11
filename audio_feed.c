/*******************************************************************************
* File Name: audio_feed.c
*
*  Description: This file contains the implementation of the feedback endpoint
*   related tasks.
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

#include "audio_feed.h"
#include "usb_comm.h"
#include "audio.h"

#include "cycfg.h"
#include "cy_sysint.h"

/*******************************************************************************
* Local Functions
*******************************************************************************/
void audio_feed_endpoint_callback(USBFS_Type *base,
                                  cy_stc_usbfs_dev_drv_context_t *context);

/*******************************************************************************
* Audio Feedback Variables
*******************************************************************************/
uint8_t audio_feed_data[AUDIO_FEEDBACK_ENDPOINT_SIZE] = {0x00, 0x00, 0x0C};


/*******************************************************************************
* Function Name: audio_feed_init
********************************************************************************
* Summary:
*   Initialize the audio feedback endpoint.
*
*******************************************************************************/
void audio_feed_init(void)
{
    /* Register SOF Callback */
    Cy_USBFS_Dev_Drv_RegisterSofCallback(CYBSP_USBDEV_HW,
                                         audio_feed_endpoint_callback,
                                         &usb_drvContext);
}

/*******************************************************************************
* Function Name: audio_feed_update_sample_rate
********************************************************************************
* Summary:
*   Set the audio streaming sample rate.
*
*******************************************************************************/
void audio_feed_update_sample_rate(uint32_t sample_rate)
{
    switch (sample_rate)
    {
        case AUDIO_SAMPLING_RATE_48KHZ:
            audio_feed_data[0] = 0x00u;
            audio_feed_data[1] = 0x00u;
            audio_feed_data[2] = 0x0Cu;
            break;

          case AUDIO_SAMPLING_RATE_44KHZ:
            audio_feed_data[0] = 0x66u;
            audio_feed_data[1] = 0x06u;
            audio_feed_data[2] = 0x0Bu;
            break;
          default:
            break;
    }
}

/*******************************************************************************
* Function Name: audio_feed_endpoint_callback
********************************************************************************
* Summary:
*   Audio feedback endpoint callback implementation. It updates the sample rate
*   based on the status of the internal I2S buffer.
*
*******************************************************************************/
void audio_feed_endpoint_callback(USBFS_Type *base, cy_stc_usbfs_dev_drv_context_t *context)
{
    uint32_t i2s_count;
    uint32_t feedback_sample_rate;
    uint8_t  feedback_data[AUDIO_FEEDBACK_ENDPOINT_SIZE];
    cy_stc_usb_dev_context_t *devContext = Cy_USBFS_Dev_Drv_GetDevContext(base, context);

    /* Only process if the enable feedback flag is set */
    if (usb_comm_enable_feedback == true)
    {
        /* Get the number of bytes in the I2S TX FIFO */
        i2s_count = Cy_I2S_GetNumInTxFifo(CYBSP_I2S_HW);

        /* Extract the current sample rate */
        feedback_sample_rate = (((uint32_t) audio_feed_data[2]) << 16) |
                               (((uint32_t) audio_feed_data[1]) << 8)  |
                               (((uint32_t) audio_feed_data[0]) << 0);

        /* Check the current I2S TX FIFO count to slightly change the sample
           rate if necessary */
        if (i2s_count < ((CYBSP_I2S_config.txFifoTriggerLevel/2) - 1))
        {
            feedback_sample_rate += AUDIO_FEED_SINGLE_SAMPLE;
        }
        else if (i2s_count > ((CYBSP_I2S_config.txFifoTriggerLevel/2) + 1))
        {
            feedback_sample_rate -= AUDIO_FEED_SINGLE_SAMPLE;
        }

        /* Update the feedback data */
        feedback_data[2] = (uint8_t) ((feedback_sample_rate & 0xFF0000) >> 16);
        feedback_data[1] = (uint8_t) ((feedback_sample_rate & 0x00FF00) >> 8);
        feedback_data[0] = (uint8_t) ((feedback_sample_rate & 0x0000FF) >> 0);

        /* Load feedback enpoint */
        Cy_USB_Dev_WriteEpNonBlocking(AUDIO_FEEDBACK_IN_ENDPOINT, feedback_data,
                                      AUDIO_FEEDBACK_ENDPOINT_SIZE, devContext);
    }
}

/* [] END OF FILE */

