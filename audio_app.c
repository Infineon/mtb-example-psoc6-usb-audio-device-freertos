/*******************************************************************************
* File Name: audio_app.c
*
* Description: This file contains the implementation of the high level audio
*  related tasks.
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
#include "audio_app.h"
#include "audio_feed.h"
#include "audio_in.h"
#include "audio_out.h"
#include "usb_comm.h"
#ifdef COMPONENT_AK4954A
    #include "ak4954a.h"
#endif
#include "touch.h"

#include "cyhal.h"
#include "cycfg.h"
#include "cybsp.h"

#include "rtos.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define MI2C_TIMEOUT_MS     10u         /* in ms */
#define MCLK_CODEC_DELAY_MS 10u         /* in ms */
#define MCLK_FREQ_HZ        12288000u   /* in Hz */
#define MCLK_DUTY_CYCLE     50.0f       /* in %  */
#define USB_CLK_DIV_48KHZ   492u
#define USB_CLK_DIV_44KHZ   452u
#define PLL_TIMEOUT_US      12000u      /* in us */

/*******************************************************************************
* Global Variables
********************************************************************************/
uint32_t audio_app_current_sample_rate = 0;
int8_t   audio_app_volume;
int8_t   audio_app_prev_volume = 0;
bool     audio_app_mute;

#ifdef COMPONENT_AK4954A
    /* Master I2C variables */
    cyhal_i2c_t mi2c;

    const cyhal_i2c_cfg_t mi2c_cfg = {
        .is_slave        = false,
        .address         = 0,
        .frequencyhal_hz = 400000
    };
#endif

/* Master Clock PWM */
cyhal_pwm_t mclk_pwm;

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void audio_app_set_clock(uint32_t sampleRate);
void audio_app_update_codec_volume(void);
void audio_app_update_sample_rate(void);

#ifdef COMPONENT_AK4954A
    cy_rslt_t mi2c_transmit(uint8_t reg_adrr, uint8_t data);
#endif

/*******************************************************************************
* Function Name: audio_app_usb_delay
********************************************************************************
* Summary:
*   Delay task used by the USB driver. Delays 1 tick time.
*
* Parameters:
*   milliseconds: number of milliseconds left to delay.
*
* Return:
*   Updated number of milliseconds left to delay.
*
*******************************************************************************/
__STATIC_INLINE int32_t audio_app_usb_delay(int32_t milliseconds)
{
    vTaskDelay(1/portTICK_PERIOD_MS);

    return (milliseconds - 1);
}

/*******************************************************************************
* Function Name: audio_app_init
********************************************************************************
* Summary:
*   Initialize all the audio related hardware, including the Audio Codec and
*   I2S interface.
*
*******************************************************************************/
void audio_app_init(void)
{
    usb_comm_interface_t interface =
    {
        .disable_in = audio_in_disable,
        .enable_in = audio_in_enable,
        .disable_out = audio_out_disable,
        .enable_out = audio_out_enable
    };

    /* Initialize the Master Clock with a PWM */
    cyhal_pwm_init(&mclk_pwm, (cyhal_gpio_t) AUDIO_APP_MCLK_PIN, NULL);
    cyhal_pwm_set_duty_cycle(&mclk_pwm, MCLK_DUTY_CYCLE, MCLK_FREQ_HZ);
    cyhal_pwm_start(&mclk_pwm);

    /* Wait for the MCLK to clock the audio codec */
    cyhal_system_delay_ms(MCLK_CODEC_DELAY_MS);
    
#ifdef COMPONENT_AK4954A
    /* Initialize the I2C Master */
    cyhal_i2c_init(&mi2c, CYBSP_I2C_SDA, CYBSP_I2C_SCL, NULL);
    cyhal_i2c_configure(&mi2c, &mi2c_cfg);

    /* Configure the AK494A codec and enable it */
    if (ak4954a_init(mi2c_transmit) != 0)
    {
        /* If failed, reset the device */
        NVIC_SystemReset();
    }
    ak4954a_activate();
    ak4954a_adjust_volume(AK4954A_HP_DEFAULT_VOLUME);
#endif

    usb_comm_init();
    usb_comm_register_interface(&interface);
    usb_comm_register_usb_callbacks();

    /* Initialize the I2S block */
    Cy_I2S_Init(CYBSP_I2S_HW, &CYBSP_I2S_config);

    /* Init the audio endpoints */
    audio_in_init();
    audio_out_init();
    audio_feed_init();

    /* Overwrite the function used internally by the USBFS to handle timeouts */
    Cy_USB_Dev_OverwriteHandleTimeout(audio_app_usb_delay, &usb_devContext);
}

/*******************************************************************************
* Function Name: audio_app_process
********************************************************************************
* Summary:
*   Main audio task. Initialize the USB communication and the audio application.
*   In the main loop, process requests to update the sample rate and change
*   the volume.
*
*******************************************************************************/
void audio_app_process(void *arg)
{
    (void) arg;

    /* Initialize the Audio application */
    audio_app_init();

    /* Enumerate the USB device */
    usb_comm_connect();

    while (1)
    {
        xEventGroupWaitBits(rtos_events, RTOS_EVENT_USB,
            pdTRUE, pdFALSE, portMAX_DELAY );

        if (0u != usb_comm_is_ready())
        {
            /* Update the sample rate */
            audio_app_update_sample_rate();

#ifdef COMPONENT_AK4954A
            /* Convert to Codec Volume */
            audio_app_update_codec_volume();
#endif

            /* Set sync bit */
            xEventGroupSetBits(rtos_events, RTOS_EVENT_SYNC);
        }
    }
}

#ifdef COMPONENT_AK4954A
/*******************************************************************************
* Function Name: audio_app_update_codec_volume
********************************************************************************
* Summary:
*   Update the audio codec volume by sending an I2C message.
*
*******************************************************************************/
void audio_app_update_codec_volume(void)
{
    int8_t  vol_usb = (int8_t) usb_comm_cur_volume[1];

    if (vol_usb > ((int8_t) AUDIO_VOL_MAX_MSB/2))
    {
        audio_app_volume = AK4954A_HP_VOLUME_MAX;
    }
    else
    {
        audio_app_volume = AUDIO_VOL_MAX_MSB/2 - vol_usb;
    }

    if (audio_app_volume != audio_app_prev_volume)
    {
        ak4954a_adjust_volume(audio_app_volume);

        audio_app_prev_volume = audio_app_volume;
    }

    /* Check if mute settings changed */
    if (usb_comm_mute != audio_app_mute)
    {
        /* Store current mute settings */
        audio_app_mute = usb_comm_mute;

        /* If mute is non-zero, then mute is active */
        if (audio_app_mute != 0)
        {
            ak4954a_adjust_volume(AK4954A_HP_MUTE_VALUE);
        }
        else
        {
            /* Otherwise, update with current volume */
            ak4954a_adjust_volume(audio_app_volume);
        }
    }
}
#endif

/*******************************************************************************
* Function Name: audio_app_update_sample_rate
********************************************************************************
* Summary:
*   Update the sample rate of the audio streaming.
*
*******************************************************************************/
void audio_app_update_sample_rate(void)
{
    /* Check if need to change sample rate. */
    if ((usb_comm_new_sample_rate != 0) &&
        (usb_comm_new_sample_rate != audio_app_current_sample_rate))
    {
        /* Capture the new sample rate */
        audio_app_current_sample_rate = usb_comm_new_sample_rate;

        /* Update feedback sample rate */
        audio_feed_update_sample_rate(audio_app_current_sample_rate);

        /* Update Audio In sample rate */
        audio_in_update_sample_rate(audio_app_current_sample_rate);

        /* Disable the I2S block */
        Cy_I2S_DisableTx(CYBSP_I2S_HW);
        Cy_I2S_DisableRx(CYBSP_I2S_HW);

#ifdef COMPONENT_AK4954A
        /* Disable the codec */
        ak4954a_deactivate();
#endif

        /* Set the new clock rate */
        audio_app_set_clock(audio_app_current_sample_rate);

#ifdef COMPONENT_AK4954A
        /* Re-enable the codec */
        ak4954a_activate();
#endif

        /* Re-enable the audio codec */
        if (usb_comm_enable_out_streaming)
        {
            Cy_I2S_EnableTx(CYBSP_I2S_HW);
            Cy_I2S_EnableRx(CYBSP_I2S_HW);
        }
        else if (usb_comm_enable_in_streaming)
        {
            Cy_I2S_EnableRx(CYBSP_I2S_HW);
        }
    }
}

/*******************************************************************************
* Function Name: audio_app_set_clock
********************************************************************************
* Summary:
*   Update the PLL clock to achieve the desired sample rate.
*
* Parameters:
*   sampleRate: new sample rate to be enforced.
*
*******************************************************************************/
void audio_app_set_clock(uint32_t sample_rate)
{
    /* Wait till the CapSense is ready */
    while (!touch_is_ready())
    {
        vTaskDelay(1);
    };

    uint32_t state = cyhal_system_critical_section_enter();

    switch (sample_rate)
    {
        case AUDIO_SAMPLING_RATE_48KHZ:
        {
            /* Disable the PLL and reconfigure its parameters */
            Cy_SysClk_PllDisable(1);

#ifndef TARGET_CY8CPROTO_062_4343W
            const cy_stc_pll_manual_config_t pllConfig48 =
            {
                .feedbackDiv  = 60u,
                .referenceDiv = 3u,
                .outputDiv    = 7u,
                .lfMode       = false,
                .outputMode   = CY_SYSCLK_FLLPLL_OUTPUT_AUTO
            };
#else
            const cy_stc_pll_manual_config_t pllConfig48 =
            {
                .feedbackDiv  = 50u,
                .referenceDiv = 7u,
                .outputDiv    = 5u,
                .lfMode       = false,
                .outputMode   = CY_SYSCLK_FLLPLL_OUTPUT_AUTO
            };
#endif
            /* Set the PLL to a frequency that generates 48 ksps */
            Cy_SysClk_PllManualConfigure(1u, &pllConfig48);

            /* Re-enable the PLL */
            Cy_SysClk_PllEnable(1, PLL_TIMEOUT_US);

            /* Disable Peripheral Clocks */
            Cy_SysClk_PeriphDisableDivider(CYBSP_USB_CLK_DIV_HW,
                                           CYBSP_USB_CLK_DIV_NUM);

            /* Update Peripheral Clocks based on the new system clock */
            Cy_SysClk_PeriphSetDivider(CYBSP_USB_CLK_DIV_HW,
                                       CYBSP_USB_CLK_DIV_NUM,
                                       USB_CLK_DIV_48KHZ);
            break;
        }
        case AUDIO_SAMPLING_RATE_44KHZ:
        {
            /* Disable the PLL and reconfigure its parameters */
            Cy_SysClk_PllDisable(1);

#ifndef TARGET_CY8CPROTO_062_4343W
            const cy_stc_pll_manual_config_t pllConfig44 =
            {
                .feedbackDiv  = 63u,
                .referenceDiv = 4u,
                .outputDiv    = 6u,
                .lfMode       = false,
                .outputMode   = CY_SYSCLK_FLLPLL_OUTPUT_AUTO
            };
#else
            const cy_stc_pll_manual_config_t pllConfig44 =
            {
                .feedbackDiv  = 63u,
                .referenceDiv = 8u,
                .outputDiv    = 6u,
                .lfMode       = false,
                .outputMode   = CY_SYSCLK_FLLPLL_OUTPUT_AUTO
            };
#endif
            /* Set the PLL to a frequency that generates 44.1 ksps */
            Cy_SysClk_PllManualConfigure(1u, &pllConfig44);

            /* Re-enable the PLL */
            Cy_SysClk_PllEnable(1, PLL_TIMEOUT_US);

            /* Disable Peripheral Clocks */
            Cy_SysClk_PeriphDisableDivider(CYBSP_USB_CLK_DIV_HW,
                                           CYBSP_USB_CLK_DIV_NUM);

            /* Update Peripheral Clocks based on the new system clock */
            Cy_SysClk_PeriphSetDivider(CYBSP_USB_CLK_DIV_HW,
                                       CYBSP_USB_CLK_DIV_NUM,
                                       USB_CLK_DIV_44KHZ);
            break;
        }
        default:
            break;
    }

    /* Enable Peripheral Clocks */
    Cy_SysClk_PeriphEnableDivider(CYBSP_USB_CLK_DIV_HW,
                                  CYBSP_USB_CLK_DIV_NUM);

    cyhal_system_critical_section_exit(state);

    /* Update baseline to compensante change in the clock */
    touch_update_baseline();

    /* Set flag to indicate that the clock was configured */
    usb_comm_clock_configured = true;
}

#ifdef COMPONENT_AK4954A
/*******************************************************************************
* Function Name: mi2c_transmit
********************************************************************************
* Summary:
*  I2C Master function to transmit data to the given address.
*
* Parameters:
*  reg_addr: address to be updated
*  data: 8-bit data to be written in the register
*
* Return:
*  cy_rslt_t - I2C master transaction error status.
*              Returns CY_RSLT_SUCCESS if succeeded.
*
*******************************************************************************/
cy_rslt_t mi2c_transmit(uint8_t reg_addr, uint8_t data)
{
    cy_rslt_t result;
    uint8_t buffer[AK4954A_PACKET_SIZE];

    buffer[0] = reg_addr;
    buffer[1] = data;

    /* Send the data over the I2C */
    result = cyhal_i2c_master_write(&mi2c, AK4954A_I2C_ADDR, buffer, AK4954A_PACKET_SIZE, MI2C_TIMEOUT_MS, true);

    return result;
}
#endif

/* [] END OF FILE */
