/*******************************************************************************
* File Name: touch.h
*
*  Description:  This file contains the function prototypes and constants
*   used in touch.c.
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
#ifndef TOUCH_H
#define TOUCH_H

#include "cycfg_capsense.h"
#include <stdbool.h>

/*******************************************************************************
* Touch Application Constants
*******************************************************************************/
#define TOUCH_PERIOD_MS                 10u

typedef enum
{
    TOUCH_DOWN          = 1 << 0,
    TOUCH_LIFT          = 1 << 1,
    TOUCH_SLIDE_RIGHT   = 1 << 2,
    TOUCH_SLIDE_LEFT    = 1 << 3,
    TOUCH_ALL        = 0xFF
} touch_event_t;

typedef struct {
    bool     button0;
    bool     button1;
    bool     slider_status;
    uint16_t slider_pos;
} touch_status_t;

/* Callback for CapSense events */
typedef void (*touch_callback_t)(uint32_t widget, touch_event_t event, uint32_t value);

/*******************************************************************************
* Touch Functions
*******************************************************************************/
void touch_init(void);
bool touch_is_ready(void);
void touch_update_baseline(void);
void touch_process(void *arg);
void touch_start_scan(void);
void touch_stop_scan(void);
void touch_get_state(touch_status_t *sensors);
void touch_register_callback(touch_callback_t callback);
void touch_enable_event(touch_event_t event, bool enable);

#endif /* TOUCH_H */

/* [] END OF FILE */
