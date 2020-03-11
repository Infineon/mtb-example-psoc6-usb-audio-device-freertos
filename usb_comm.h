/*****************************************************************************
* File Name: usb_comm.h
*
*
* Description: This file contains the function prototypes and constants used in
*  usb_comm.c.
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
#ifndef USB_COMM_H
#define USB_COMM_H

#include <stdint.h>
#include <stdbool.h>

#include "cy_usb_dev.h"
#include "cy_usb_dev_audio.h"
#include "cy_usb_dev_audio_descr.h"

#include "audio.h"

/*******************************************************************************
* USB Communication Strucutres
*******************************************************************************/
typedef void (* usb_comm_interface_function_t)(void);

typedef struct
{
    usb_comm_interface_function_t enable_out;
    usb_comm_interface_function_t enable_in;
    usb_comm_interface_function_t disable_out;
    usb_comm_interface_function_t disable_in;
} usb_comm_interface_t;

/*******************************************************************************
* USB Communication Extern Global Variables
*******************************************************************************/
extern uint8_t usb_comm_mute;
extern uint8_t usb_comm_cur_volume[AUDIO_VOLUME_SIZE];
extern uint8_t usb_comm_min_volume[AUDIO_VOLUME_SIZE];
extern uint8_t usb_comm_max_volume[AUDIO_VOLUME_SIZE];
extern uint8_t usb_comm_res_volume[AUDIO_VOLUME_SIZE];

extern volatile uint32_t usb_comm_new_sample_rate;
extern volatile bool     usb_comm_enable_out_streaming;
extern volatile bool     usb_comm_enable_in_streaming;
extern volatile bool     usb_comm_out_streaming_start;
extern volatile bool     usb_comm_in_streaming_start;
extern volatile bool     usb_comm_out_streaming_stop;
extern volatile bool     usb_comm_in_streaming_stop;
extern volatile bool     usb_comm_enable_feedback;
extern volatile bool     usb_comm_clock_configured;

/* USBFS Context structures */
extern cy_stc_usbfs_dev_drv_context_t  usb_drvContext;
extern cy_stc_usb_dev_context_t        usb_devContext;
extern cy_stc_usb_dev_audio_context_t  usb_audioContext;

/*******************************************************************************
* USB Communication Functions
*******************************************************************************/
void     usb_comm_init(void);
void     usb_comm_connect(void);
bool     usb_comm_is_ready(void);
void     usb_comm_register_interface(usb_comm_interface_t *interface);
void     usb_comm_register_usb_callbacks(void);
uint32_t usb_comm_get_sample_rate(uint32_t endpoint);

#endif /* USB_COMM_H */

/* [] END OF FILE */
