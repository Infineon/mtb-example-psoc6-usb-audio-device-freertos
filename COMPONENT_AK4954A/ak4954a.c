/*******************************************************************************
* File Name: ak4954a.c
*
* Description: This file contains the AK4954a codec control APIs.
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

#include "ak4954a.h"
#include "stdbool.h"

#define I2C_WRITE_OPERATION        (0x00)

ak4954a_transmit_callback   ak4954a_transmit;

/*******************************************************************************
* Function Name: ak4954a_init
********************************************************************************
* Summary:
*   Initializes the codec with default settings.
*
* Parameters:  
*    None
*
* Return:
*   uint32_t - I2C master transaction error status 
*
*******************************************************************************/
uint32_t ak4954a_init(ak4954a_transmit_callback callback)
{
    uint32_t ret;
    
    ak4954a_transmit = callback;
   
    /* Clear Power Managament 1 register (dummy write) */
    ret = ak4954a_transmit(AK4954A_REG_PWR_MGMT1, 0x00);
    if (ret) return ret;

    /* Clear Power Managament 1 register */
    ret = ak4954a_transmit(AK4954A_REG_PWR_MGMT1, 0x00);
    if (ret) return ret;
    
    /* Set the data alignment */
    ret = ak4954a_transmit(AK4954A_REG_MODE_CTRL1, AK4954A_DEF_DATA_ALIGNMENT);
    if (ret) return ret;
    
    /* Set sample rate */
    ret = ak4954a_transmit(AK4954A_REG_MODE_CTRL2, AK4954A_DEF_SAMPLING_RATE |
                                                   AK4954A_MODE_CTRL2_FS_48kHz);
    if (ret) return ret;
    
    /* Set MPWR pin Power Management */
    ret = ak4954a_transmit(AK4954A_REG_SIG_SEL1, AK4954A_SIG_SEL1_PMMP |
                                                 AK4954A_SIG_SEL1_MGAIN_0dB);
    if (ret) return ret;
    
    /* Clear Digital Filter Mode register */
    ret = ak4954a_transmit(AK4954A_REG_DIG_FILT_MODE, 0x00);
    if (ret) return ret;

    /* Mute right channel [Not used] */
    ret = ak4954a_transmit(AK4954A_REG_MODE_CTRL3, 0x00);
    if (ret) return ret;
    ret = ak4954a_transmit(AK4954A_REG_RCH_IN_VOL, 0x00);
    if (ret) return ret;
    ret = ak4954a_transmit(AK4954A_REG_LCH_IN_VOL, 0x00);

    return ret;  
}

/*******************************************************************************
* Function Name: ak4954a_adjust_volume
********************************************************************************
* Summary:
*   This function updates the volume of both the left and right channels of the
*     headphone output.
*
*
* Parameters:  
*    volume - Steps of 0.5dB, where:
*            Minimum volume: -65.5dB (0x8F)
*            Maximum volume:  +6.0dB (0x00)
*            Mute: (0x90~0xFF)
*
* Return:
*   uint32_t - I2C master transaction error status
*
*******************************************************************************/
uint32_t ak4954a_adjust_volume(uint8_t volume)
{
    uint32_t ret;
        
    ret = ak4954a_transmit(AK4954A_REG_LCH_DIG_VOL, volume);
    if (ret) return ret;
    return ak4954a_transmit(AK4954A_REG_RCH_DIG_VOL, volume);
}

/*******************************************************************************
* Function Name: ak4954a_activate
********************************************************************************
* Summary:
*   Activates the codec - This function is called in conjunction with 
*   ak4954A_deactivate API after successful configuration update of the codec.
*
* Parameters:  
*    None
*
* Return:
*   uint32_t - I2C master transaction error status
*
*******************************************************************************/
uint32_t ak4954a_activate(void)
{
    uint32_t ret;
    
    /* Enable Power Management DAC */
    ret = ak4954a_transmit(AK4954A_REG_PWR_MGMT1,
                           AK4954A_PWR_MGMT1_PMDAC | AK4954A_PWR_MGMT1_PMVCM |
                           AK4954A_PWR_MGMT1_PMADL | AK4954A_PWR_MGMT1_PMADR |
                           AK4954A_PWR_MGMT1_PMPFIL);
    if (ret) return ret;
    
    /* Enable Left/Right Channels */
    ret = ak4954a_transmit(AK4954A_REG_PWR_MGMT2, 
                           AK4954A_PWR_MGMT2_PMHPL | AK4954A_PWR_MGMT2_PMHPR);
    return ret;
}

/*******************************************************************************
* Function Name: ak4954a_deactivate
********************************************************************************
* Summary:
*   Deactivates the codec - the configuration is retained, just the codec 
*   input/outputs are disabled. The function should be called before changing 
*   any setting in the codec over I2C
*
* Parameters:  
*    None
*
* Return:
*   uint32_t - I2C master transaction error status
*
*******************************************************************************/
uint32_t ak4954a_deactivate(void)
{
    uint32_t ret;
   
    /* Disable Left/Right Channels */
    ret = ak4954a_transmit(AK4954A_REG_PWR_MGMT2, 0x00);
    
    /* Disable Power Management DAC */
    ret = ak4954a_transmit(AK4954A_REG_PWR_MGMT1, AK4954A_PWR_MGMT1_PMVCM);
    return ret;
}

/* [] END OF FILE */
