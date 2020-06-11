/*****************************************************************************
* File Name: audio_app.h
*
* Description: This file contains the function prototypes and constants used in
*  audio_app.c.
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
#ifndef AUDIO_APP_H
#define AUDIO_APP_H

#include "cyhal.h"
#include "audio.h"

/*******************************************************************************
* Constants
*******************************************************************************/
#define AUDIO_APP_MCLK_PIN          P5_0

#define PC_VOLUME_MSB_CODEC_OFFSET  64
#define PC_VOLUME_CODEC_COEFF       4096

/*******************************************************************************
* Externs
*******************************************************************************/
extern cyhal_i2s_t i2s;

/*******************************************************************************
* Functions
*******************************************************************************/
void audio_app_init(void);
void audio_app_process(void *arg);

#endif /* AUDIO_APP_H */
