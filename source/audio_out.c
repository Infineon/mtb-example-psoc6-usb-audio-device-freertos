/*******************************************************************************
* File Name: audio_out.c
*
*  Description: This file contains the Audio Out path configuration and
*               processing code
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

#include "audio_out.h"
#include "audio_app.h"
#include "audio.h"
#include "usb_comm.h"

#include "cyhal.h"
#include "cycfg.h"
#include "cy_sysint.h"

#include "rtos.h"

#include "cy_device_headers.h"

/*******************************************************************************
* Local Functions
*******************************************************************************/
void audio_out_endpoint_callback(USBFS_Type *base,
                                 uint32_t endpoint,
                                 uint32_t errorType,
                                 cy_stc_usbfs_dev_drv_context_t *context);

void convert_24_to_32_array(uint8_t *src, uint8_t *dst, uint32_t length);

/*******************************************************************************
* Audio Out Variables
*******************************************************************************/
/* USB OUT buffer data for Audio OUT endpoint */
CY_USB_DEV_ALLOC_ENDPOINT_BUFFER(audio_out_usb_buffer, AUDIO_OUT_ENDPOINT_SIZE);

/* PCM Intermediary buffer (32-bits) */
uint8_t audio_out_to_i2s_tx[4 * AUDIO_OUT_ENDPOINT_SIZE/AUDIO_SAMPLE_DATA_SIZE];

/*******************************************************************************
* Function Name: audio_out_init
********************************************************************************
* Summary:
*   Initialize the audio OUT endpoint by setting up the DMAs involved and the
*   I2S block.
*
*******************************************************************************/
void audio_out_init(void)
{
    /* Register Data Endpoint Callbacks */
    Cy_USBFS_Dev_Drv_RegisterEndpointCallback(CYBSP_USBDEV_HW,
                                              AUDIO_STREAMING_OUT_ENDPOINT,
                                              audio_out_endpoint_callback,
                                              &usb_drvContext);
}

/*******************************************************************************
* Function Name: audio_out_enable
********************************************************************************
* Summary:
*   Start a playing session.
*
*******************************************************************************/
void audio_out_enable(void)
{
    BaseType_t xHigherPriorityTaskWoken, xResult;

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. */
    xHigherPriorityTaskWoken = pdFALSE;

    xResult  = xEventGroupSetBitsFromISR(rtos_events, RTOS_EVENT_OUT,
                                         &xHigherPriorityTaskWoken);

    /* Was the message posted successfully? */
    if( xResult != pdFAIL )
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
        switch should be requested.  The macro used is port specific and will
        be either portYIELD_FROM_ISR() or portEND_SWITCHING_ISR() - refer to
        the documentation page for the port being used. */
      portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
    }
}

/*******************************************************************************
* Function Name: audio_out_disable
********************************************************************************
* Summary:
*   Stop a playing session.
*
*******************************************************************************/
void audio_out_disable(void)
{
    /* Stop the I2S TX */
    cyhal_i2s_stop_tx(&i2s);

#ifdef COMPONENT_AK4954A
    /* If not audio IN streaming, stop RX as well */
    if (false == usb_comm_enable_in_streaming)
    {
        cyhal_i2s_stop_rx(&i2s);
    }
#endif
}

/*******************************************************************************
* Function Name: audio_out_process
********************************************************************************
* Summary:
*   Main task for the audio out endpoint. Check if it should start a playing
*   session.
*
*******************************************************************************/
void audio_out_process(void *arg)
{
    (void) arg;

    while (1)
    {
        /* Only process if a request to stream out data is set and the clock is
           in sync with the desired sample rate */
        xEventGroupWaitBits(rtos_events,
                            RTOS_EVENT_OUT | RTOS_EVENT_SYNC,
                            pdFALSE, pdTRUE, portMAX_DELAY);

        if (usb_comm_clock_configured)
        {

            /* Start I2S Tx */
            Cy_I2S_ClearTxFifo(i2s.base);

    #ifdef COMPONENT_AK4954A
            if (usb_comm_enable_in_streaming == false)
            {
                cyhal_i2s_start_rx(&i2s);
            }
    #endif

            /* Arm the USB to receive data from host */
            Cy_USB_Dev_StartReadEp(AUDIO_STREAMING_OUT_ENDPOINT, &usb_devContext);

            /* Clear Event OUT flag */
            xEventGroupClearBits(rtos_events, RTOS_EVENT_OUT);
        }
    }
}

/*******************************************************************************
* Function Name: audio_out_endpoint_callback
********************************************************************************
* Summary:
*   Audio OUT endpoint callback implementation. It enables the Audio OUT DMA to
*   stream an audio frame.
*
*******************************************************************************/
void audio_out_endpoint_callback(USBFS_Type *base,
                                 uint32_t endpoint,
                                 uint32_t errorType,
                                 cy_stc_usbfs_dev_drv_context_t *context)
{
    uint32_t count;
    uint32_t data_to_write;

    (void) errorType;
    (void) endpoint,
    (void) context;
    (void) base;

    /* Check if should keep playing */
    if (usb_comm_enable_out_streaming == true)
    {
        /* Read audio data from the OUT endpoint */
        Cy_USB_Dev_ReadEpNonBlocking(AUDIO_STREAMING_OUT_ENDPOINT,
                                     audio_out_usb_buffer, AUDIO_OUT_ENDPOINT_SIZE,
                                     &count, &usb_devContext);

        data_to_write = count / AUDIO_SAMPLE_DATA_SIZE;

        /* Convert USB array (24-bit) to I2S array (32-bit) */
        convert_24_to_32_array(audio_out_usb_buffer, audio_out_to_i2s_tx, data_to_write);

        /* Write data to I2S Tx */
        cyhal_i2s_write(&i2s, audio_out_to_i2s_tx, (size_t *) &data_to_write);

        /* Start the I2S TX if disabled */
        if ((Cy_I2S_GetCurrentState(i2s.base) & CY_I2S_TX_START) == 0)
        {
            cyhal_i2s_start_tx(&i2s);
        }
    }
}

/*******************************************************************************
* Function Name: convert_24_to_32_array
********************************************************************************
* Summary:
*   Convert a 24-bit array to 32-bit array.
*
*******************************************************************************/
void convert_24_to_32_array(uint8_t *src, uint8_t *dst, uint32_t length)
{
    while (0u != length--)
    {
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = *(src++);
        *(dst++) = 0;
    }
}

/* [] END OF FILE */
