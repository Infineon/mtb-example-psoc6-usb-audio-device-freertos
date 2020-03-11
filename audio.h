/*****************************************************************************
* File Name: audio.h
*
*
* Description: This file contains the constants mapped to the USB descriptor.
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
#ifndef AUDIO_H
#define AUDIO_H

/*******************************************************************************
* Constants from USB Audio Descriptor
*******************************************************************************/
#define AUDIO_OUT_ENDPOINT_SIZE         (294U)
#define AUDIO_IN_ENDPOINT_SIZE          (294U)
#define AUDIO_FEEDBACK_ENDPOINT_SIZE    (3U)

#define AUDIO_FRAME_DATA_SIZE           (96u)
#define AUDIO_DELTA_VALUE               (2u)
#define AUDIO_MAX_DATA_SIZE             (AUDIO_FRAME_DATA_SIZE + AUDIO_DELTA_VALUE)

#define AUDIO_CONTROL_INTERFACE         (0x00U)
#define AUDIO_CONTROL_IN_ENDPOINT       (6U)
#define AUDIO_CONTROL_FEATURE_UNIT_IDX  (0x02U)
#define AUDIO_CONTROL_FEATURE_UNIT      ((AUDIO_CONTROL_FEATURE_UNIT_IDX << 8U) | (AUDIO_CONTROL_INTERFACE))

#define AUDIO_STREAMING_OUT_INTERFACE   (1U)
#define AUDIO_STREAMING_OUT_ALTERNATE   (1U)
#define AUDIO_STREAMING_IN_INTERFACE    (2U)
#define AUDIO_STREAMING_IN_ALTERNATE    (1U)

#define AUDIO_STREAMING_OUT_ENDPOINT    (1U)
#define AUDIO_STREAMING_IN_ENDPOINT     (2U)
#define AUDIO_FEEDBACK_IN_ENDPOINT      (3U)

#define AUDIO_STREAMING_OUT_ENDPOINT_ADDR   (0x01U)
#define AUDIO_STREAMING_IN_ENDPOINT_ADDR    (0x82U)
#define AUDIO_FEEDBACK_IN_ENDPOINT_ADDR     (0x83U)

#define AUDIO_FEED_SINGLE_SAMPLE            (0x000800U)

#define AUDIO_STREAMING_EPS_NUMBER          (0x2U)
#define AUDIO_SAMPLE_FREQ_SIZE              (3U)
#define AUDIO_SAMPLE_DATA_SIZE              (3U)

#define AUDIO_FEATURE_UNIT_MASTER_CHANNEL   (0U)

#define AUDIO_VOLUME_SIZE   (2U)

#define AUDIO_VOL_MIN_MSB   (0x80u)
#define AUDIO_VOL_MIN_LSB   (0x01u)
#define AUDIO_VOL_MAX_MSB   (0x7Fu)
#define AUDIO_VOL_MAX_LSB   (0xFFu)
#define AUDIO_VOL_RES_MSB   (0x00u)
#define AUDIO_VOL_RES_LSB   (0x01u)

#define AUDIO_SAMPLING_RATE_48KHZ   (48000U)
#define AUDIO_SAMPLING_RATE_44KHZ   (44100U)
#define AUDIO_SAMPLING_RATE_32KHZ   (32000U)
#define AUDIO_SAMPLING_RATE_22KHZ   (22050U)
#define AUDIO_SAMPLING_RATE_16KHZ   (16000U)

#endif /* AUDIO_H */

/* [] END OF FILE */
