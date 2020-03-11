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
#include "audio.h"
#include "usb_comm.h"

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

/*******************************************************************************
* Audio In Variables
*******************************************************************************/
/* USB IN buffer data for Audio IN endpoint */
CY_USB_DEV_ALLOC_ENDPOINT_BUFFER(audio_out_usb_buffer, AUDIO_OUT_ENDPOINT_SIZE);

/* PCM Intermediary variable (32-bits) */
uint8_t audio_out_to_i2s_tx[4];

/* Controls the remaining number of bytes to be written in the audio_out_to_i2s_tx */
volatile uint32_t audio_out_remaining_bytes = 0;

/* Audio IN flags */
volatile bool audio_out_start_playing = false;
volatile bool audio_out_stop_playing = false;
volatile bool audio_out_is_playing    = false;

extern volatile bool audio_in_is_recording;

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

    /* Initialize the DMAs */
    Cy_DMA_Descriptor_Init(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_0_config);
    Cy_DMA_Channel_Init(CYBSP_DMA_USB_OUT_HW, CYBSP_DMA_USB_OUT_CHANNEL, &CYBSP_DMA_USB_OUT_channelConfig);
    Cy_DMA_Enable(CYBSP_DMA_USB_OUT_HW);
    Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_0, (void *) audio_out_usb_buffer);
    Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_USB_OUT_Descriptor_0, (void *) audio_out_to_i2s_tx);

    Cy_DMA_Descriptor_Init(&CYBSP_DMA_I2S_TX_Descriptor_0, &CYBSP_DMA_I2S_TX_Descriptor_0_config);
    Cy_DMA_Channel_Init(CYBSP_DMA_I2S_TX_HW, CYBSP_DMA_I2S_TX_CHANNEL, &CYBSP_DMA_I2S_TX_channelConfig);
    Cy_DMA_Enable(CYBSP_DMA_I2S_TX_HW);
    Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_I2S_TX_Descriptor_0, (void *) audio_out_to_i2s_tx);
    Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_I2S_TX_Descriptor_0, (void *) &CYBSP_I2S_HW->TX_FIFO_WR);
    Cy_DMA_Channel_Enable(CYBSP_DMA_I2S_TX_HW, CYBSP_DMA_I2S_TX_CHANNEL);
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
    /* Disable the I2S TX */
    Cy_I2S_DisableTx(CYBSP_I2S_HW);

    /* If not audio IN streaming, disable RX as well */
    if (false == usb_comm_enable_in_streaming)
    {
        Cy_I2S_DisableRx(CYBSP_I2S_HW);
    }
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

        /* Reset the remaining bytes variable */
        audio_out_remaining_bytes = 0;

        /* Clear I2S RX FIFO */
        Cy_I2S_ClearTxFifo(CYBSP_I2S_HW);

        /* Enable I2S TX */
        Cy_I2S_EnableTx(CYBSP_I2S_HW);

        if (usb_comm_enable_in_streaming == false)
        {
            Cy_I2S_EnableRx(CYBSP_I2S_HW);
        }

        /* Arm the USB to receive data from host */
        Cy_USB_Dev_StartReadEp(AUDIO_STREAMING_OUT_ENDPOINT, &usb_devContext);

        /* Clear Event OUT flag */
        xEventGroupClearBits(rtos_events, RTOS_EVENT_OUT);
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
    uint32_t mod;

    cy_stc_usb_dev_context_t *devContext = Cy_USBFS_Dev_Drv_GetDevContext(base, context);

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
                                     &count, devContext);

        /* Check for any remaining byte of a sample, which has 3 bytes */
        mod = count % 3;

        /* Setup the DMAs to trigger based on how much data is being read */
        if (audio_out_remaining_bytes == 0)
        {
            Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_0, count/3);
            Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_0, &audio_out_usb_buffer[0]);

            if (mod == 1)
            {
                /* Pos Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_1);
                Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_1, &audio_out_usb_buffer[count+audio_out_remaining_bytes-mod]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_1, 1);
                audio_out_remaining_bytes = 2;
            }
            else if (mod == 2)
            {
                /* Pos Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_1);
                Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_1, &audio_out_usb_buffer[count+audio_out_remaining_bytes-mod]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_1, 2);
                audio_out_remaining_bytes = 1;
            }
            else
            {
                /* Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_0);
                audio_out_remaining_bytes = 0;
            }

            Cy_DMA_Channel_SetDescriptor(CYBSP_DMA_USB_OUT_HW, CYBSP_DMA_USB_OUT_CHANNEL, &CYBSP_DMA_USB_OUT_Descriptor_0);
            Cy_DMA_Channel_Enable(CYBSP_DMA_USB_OUT_HW, CYBSP_DMA_USB_OUT_CHANNEL);
        }
        else if (audio_out_remaining_bytes == 1)
        {
            Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_0, count/3);
            Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_0, &audio_out_usb_buffer[1]);

            if (mod == 1)
            {
                /* Pre Descriptor Configuration */
                Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_USB_OUT_Descriptor_2, &audio_out_to_i2s_tx[2]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_2, 1);

                /* Pos Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_1);
                Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_1, &audio_out_usb_buffer[count+audio_out_remaining_bytes-mod]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_1, 1);
                audio_out_remaining_bytes = 2;
            }
            else if (mod == 2)
            {
                /* Pre Descriptor Configuration */
                Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_USB_OUT_Descriptor_2, &audio_out_to_i2s_tx[2]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_2, 1);

                /* Pos Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_1);
                Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_1, &audio_out_usb_buffer[count+audio_out_remaining_bytes-mod]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_1, 2);
                audio_out_remaining_bytes = 1;
            }
            else
            {
                /* Pre Descriptor Configuration */
                Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_USB_OUT_Descriptor_2, &audio_out_to_i2s_tx[2]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_2, 1);

                /* Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_0);
                audio_out_remaining_bytes = 0;
            }

            Cy_DMA_Channel_SetDescriptor(CYBSP_DMA_USB_OUT_HW, CYBSP_DMA_USB_OUT_CHANNEL, &CYBSP_DMA_USB_OUT_Descriptor_2);
            Cy_DMA_Channel_Enable(CYBSP_DMA_USB_OUT_HW, CYBSP_DMA_USB_OUT_CHANNEL);
        }
        else /* audio_out_remaining_bytes == 2 */
        {
            Cy_DMA_Descriptor_SetYloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_0, count/3);
            Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_0, &audio_out_usb_buffer[2]);

            if (mod == 1)
            {
                /* Pre Descriptor Configuration */
                Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_USB_OUT_Descriptor_2, &audio_out_to_i2s_tx[2]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_2, 2);

                /* Pos Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_1);
                Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_1, &audio_out_usb_buffer[count+audio_out_remaining_bytes-mod]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_1, 1);
                audio_out_remaining_bytes = 2;
            }
            else if (mod == 2)
            {
                /* Pre Descriptor Configuration */
                Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_USB_OUT_Descriptor_2, &audio_out_to_i2s_tx[1]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_2, 1);

                /* Pos Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_1);
                Cy_DMA_Descriptor_SetSrcAddress(&CYBSP_DMA_USB_OUT_Descriptor_1, &audio_out_usb_buffer[count+audio_out_remaining_bytes-mod]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_1, 2);
                audio_out_remaining_bytes = 1;
            }
            else
            {
                /* Pre Descriptor Configuration */
                Cy_DMA_Descriptor_SetDstAddress(&CYBSP_DMA_USB_OUT_Descriptor_2, &audio_out_to_i2s_tx[1]);
                Cy_DMA_Descriptor_SetXloopDataCount(&CYBSP_DMA_USB_OUT_Descriptor_2, 2);

                /* Descriptor Configuration */
                Cy_DMA_Descriptor_SetNextDescriptor(&CYBSP_DMA_USB_OUT_Descriptor_0, &CYBSP_DMA_USB_OUT_Descriptor_0);
                audio_out_remaining_bytes = 0;
            }

            Cy_DMA_Channel_SetDescriptor(CYBSP_DMA_USB_OUT_HW, CYBSP_DMA_USB_OUT_CHANNEL, &CYBSP_DMA_USB_OUT_Descriptor_2);
            Cy_DMA_Channel_Enable(CYBSP_DMA_USB_OUT_HW, CYBSP_DMA_USB_OUT_CHANNEL);
        }

        /* Trigger the DMA USB OUT */
        #ifdef _PSOC6_01_CONFIG_H_
            Cy_TrigMux_SwTrigger((uint32_t) TRIG1_OUT_CPUSS_DW1_TR_IN5, CY_TRIGGER_TWO_CYCLES);
        #endif

        #ifdef _PSOC6_02_CONFIG_H_
            Cy_TrigMux_SwTrigger((uint32_t) TRIG_OUT_MUX_1_PDMA1_TR_IN5, CY_TRIGGER_TWO_CYCLES);
        #endif
    }

    Cy_USB_Dev_StartReadEp(AUDIO_STREAMING_OUT_ENDPOINT, devContext);
}


/* [] END OF FILE */
