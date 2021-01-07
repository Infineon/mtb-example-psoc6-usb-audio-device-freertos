/*******************************************************************************
* File Name: rtos.h
*
*  Description:  This file contains the function prototypes and constants
*   related to the RTOS.
*
******************************************************************************
* (c) 2019-2020, Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* ("Software"), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries ("Cypress") and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software ("EULA").
*
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress's integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer of such
* system or application assumes all risk of such use and in doing so agrees to
* indemnify Cypress against all liability.
*****************************************************************************/
#ifndef RTOS_H
#define RTOS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

/***************************************
*    RTOS Constants
***************************************/
#define RTOS_QUEUE_SIZE     8u
#define RTOS_STACK_DEPTH    256u
#define RTOS_TASK_PRIORITY  1u

#define RTOS_EVENT_IN       0x01u
#define RTOS_EVENT_OUT      0x02u
#define RTOS_EVENT_SYNC     0x04u
#define RTOS_EVENT_USB      0x08u

/***************************************
*    Event Group Handler
***************************************/
extern EventGroupHandle_t rtos_events;

/***************************************
*    Task Handlers
***************************************/
extern TaskHandle_t rtos_audio_app_task;
extern TaskHandle_t rtos_audio_in_task;
extern TaskHandle_t rtos_audio_out_task;
extern TaskHandle_t rtos_touch_task;

#endif

/* [] END OF FILE */

