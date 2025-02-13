/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products.
* No other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws. 
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED
* OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY
* LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE FOR ANY DIRECT,
* INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR
* ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability 
* of this software. By using this software, you agree to the additional terms and conditions found by accessing the 
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2021, 2023 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/

/***********************************************************************************************************************
* File Name        : Config_TAU0_4.c
* Component Version: 1.4.2
* Device(s)        : R5F12068xSP
* Description      : This file implements device driver for Config_TAU0_4.
***********************************************************************************************************************/
/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include "r_cg_macrodriver.h"
#include "r_cg_userdefine.h"
#include "Config_TAU0_4.h"
/* Start user code for include. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Pragma directive
***********************************************************************************************************************/
/* Start user code for pragma. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
Global variables and functions
***********************************************************************************************************************/
/* For TAU0_ch4 pulse measurement */
extern volatile uint32_t g_tau0_ch4_width;
/* Start user code for global. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */

/***********************************************************************************************************************
* Function Name: R_Config_TAU0_4_Create
* Description  : This function initializes the TAU0 channel4 module.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAU0_4_Create(void)
{
    TPS0 &= _FFF0_TAU_CKM0_CLEAR;
    TPS0 |= _0000_TAU_CKM0_FCLK_0;
    /* Stop channel 4 */
    TT0 |= _0010_TAU_CH4_STOP_TRG_ON;
    /* Mask channel 4 interrupt */
    TMMK04 = 1U;    /* disable INTTM04 interrupt */
    TMIF04 = 0U;    /* clear INTTM04 interrupt flag */
    /* Set INTTM04 low priority */
    TMPR104 = 1U;
    TMPR004 = 1U;
    /* TAU04 is used to measure input pulse interval */
    NFEN1 &= (uint8_t)~_10_TAU_CH4_NOISE_ON;
    TMR04 = _0000_TAU_CLOCK_SELECT_CKM0 | _0000_TAU_CLOCK_MODE_CKS | _0000_TAU_COMBINATION_SLAVE | 
            _0100_TAU_TRIGGER_TIMN_VALID | _0080_TAU_TIMN_EDGE_BOTH_LOW | _0004_TAU_MODE_CAPTURE | 
            _0000_TAU_START_INT_UNUSED;
    TOM0 &= (uint16_t)~_0010_TAU_CH4_SLAVE_OUTPUT;
    TOL0 &= (uint16_t)~_0010_TAU_CH4_OUTPUT_LEVEL_L;
    TO0 &= (uint16_t)~_0010_TAU_CH4_OUTPUT_VALUE_1;
    TOE0 &= (uint16_t)~_0010_TAU_CH4_OUTPUT_ENABLE;
    /* Set TI04 pin */
    PMC2 &= 0xF7U;
    PM2 |= 0x08U;

    R_Config_TAU0_4_Create_UserInit();
}

/***********************************************************************************************************************
* Function Name: R_Config_TAU0_4_Start
* Description  : This function starts the TAU0 channel4 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAU0_4_Start(void)
{
    TMIF04 = 0U;    /* clear INTTM04 interrupt flag */
    TMMK04 = 0U;    /* enable INTTM04 interrupt */
    TS0 |= _0010_TAU_CH4_START_TRG_ON;
}

/***********************************************************************************************************************
* Function Name: R_Config_TAU0_4_Stop
* Description  : This function stops the TAU0 channel4 counter.
* Arguments    : None
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAU0_4_Stop(void)
{
    TT0 |= _0010_TAU_CH4_STOP_TRG_ON;
    /* Mask channel 4 interrupt */
    TMMK04 = 1U;    /* disable INTTM04 interrupt */
    TMIF04 = 0U;    /* clear INTTM04 interrupt flag */
}

/***********************************************************************************************************************
* Function Name: R_Config_TAU0_4_Get_PulseWidth
* Description  : This function measures TAU0 channel 4 input pulse width.
* Arguments    : width -
*                    the address where to write the input pulse width
* Return Value : None
***********************************************************************************************************************/
void R_Config_TAU0_4_Get_PulseWidth(uint32_t * const width)
{
    /* For channel 4 pulse measurement */
    *width = g_tau0_ch4_width;
}

/* Start user code for adding. Do not edit comment generated here */
/* End user code. Do not edit comment generated here */
