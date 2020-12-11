/* ****************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 * $Date: 2016-10-10 15:10:41 -0500 (Mon, 10 Oct 2016) $
 * $Revision: 24650 $
 *
 *************************************************************************** */

/**
 * @file mxc_sys.c
 * @brief      System layer driver.
 * @details    This driver is used to control the system layer of the device.
 */

/* **** Includes **** */
#include <stddef.h>
#include "mxc_device.h"
#include "mxc_assert.h"
#include "mxc_sys.h"
#include "mxc_delay.h"
#include "lpgcr_regs.h"
#include "gcr_regs.h"
#include "fcr_regs.h"
#include "mcr_regs.h"

/**
 * @ingroup mxc_sys
 * @{
 */

/* **** Definitions **** */
#define MXC_SYS_CLOCK_TIMEOUT       MSEC(1)

/* **** Globals **** */

/* **** Functions **** */

/* ************************************************************************** */
int MXC_SYS_IsClockEnabled(mxc_sys_periph_clock_t clock)
{
    /* The mxc_sys_periph_clock_t enum uses enum values that are the offset by 32 and 64 for the perckcn1 register. */
    if (clock > 63) {
        clock -= 64;
        return !(MXC_LPGCR->pclkdis & (0x1 << clock));
    }
    else if (clock > 31) {
        clock -= 32;
        return !(MXC_GCR->pclkdis1 & (0x1 << clock));
    }
    else {
        return !(MXC_GCR->pclkdis0 & (0x1 << clock));
    }
}

/* ************************************************************************** */
void MXC_SYS_ClockDisable(mxc_sys_periph_clock_t clock)
{
    /* The mxc_sys_periph_clock_t enum uses enum values that are the offset by 32 and 64 for the perckcn1 register. */
    if (clock > 63) {
        clock -= 64;
        MXC_LPGCR->pclkdis |= (0x1 << clock);
    }
    else if (clock > 31) {
        clock -= 32;
        MXC_GCR->pclkdis1  |= (0x1 << clock);
    }
    else {
        MXC_GCR->pclkdis0  |= (0x1 << clock);
    }
}

/* ************************************************************************** */
void MXC_SYS_ClockEnable(mxc_sys_periph_clock_t clock)
{
    /* The mxc_sys_periph_clock_t enum uses enum values that are the offset by 32 and 64 for the perckcn1 register. */
    if (clock > 63) {
        clock -= 64;
        MXC_LPGCR->pclkdis &= ~(0x1 << clock);
    }
    else if (clock > 31) {
        clock -= 32;
        MXC_GCR->pclkdis1  &= ~(0x1 << clock);
    }
    else {
        MXC_GCR->pclkdis0  &= ~(0x1 << clock);
    }
}
/* ************************************************************************** */
void MXC_SYS_RTCClockEnable()
{
    MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_ERTCO_EN;
}

/* ************************************************************************** */
int MXC_SYS_RTCClockDisable(void)
{
    /* Check that the RTC is not the system clock source */
    if ((MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_SYSCLK_SEL) != MXC_S_GCR_CLKCTRL_SYSCLK_SEL_ERTCO) {
        MXC_GCR->clkctrl &= ~MXC_F_GCR_CLKCTRL_ERTCO_EN;
        return E_NO_ERROR;
    }
    else {
        return E_BAD_STATE;
    }
}

/******************************************************************************/
int MXC_SYS_ClockSourceEnable(mxc_sys_system_clock_t clock)
{
    switch (clock) {
    case MXC_SYS_CLOCK_IPO:
        MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_IPO_EN;
        return MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_IPO_RDY);
        break;
        
    case MXC_SYS_CLOCK_IBRO:
        MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_IBRO_EN;
        return MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_IBRO_RDY);
        break;
#ifdef MXC_SYS_CLOCK_ISO // AI85 only    TODO: seperate ai85 and me17 files
        
    case MXC_SYS_CLOCK_ISO:
        MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_ISO_EN;
        return MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_ISO_RDY);
        break;
#endif
        
    case MXC_SYS_CLOCK_EXTCLK:
        // MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_EXTCLK_EN;
        // return MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_EXTCLK_RDY);
        return E_NOT_SUPPORTED;
        break;
        
    case MXC_SYS_CLOCK_INRO:
        // The 80k clock is always enabled
        return MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_INRO_RDY);
        break;
        
#ifdef MXC_SYS_CLOCK_ERFO // ME17 only
        
    case MXC_SYS_CLOCK_ERFO:
        MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_ERFO_EN;
        return MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_ERFO_RDY);
        break;
#endif
        
    case MXC_SYS_CLOCK_ERTCO:
        MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_ERTCO_EN;
        return MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_ERTCO_RDY);
        break;
        
    default:
        return E_BAD_PARAM;
        break;
    }
}

/******************************************************************************/
int MXC_SYS_ClockSourceDisable(mxc_sys_system_clock_t clock)
{
    uint32_t current_clock;
    
    current_clock = MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_SYSCLK_SEL;
    
    // Don't turn off the clock we're running on
    if (clock == current_clock) {
        return E_BAD_PARAM;
    }
    
    switch (clock) {
    case MXC_SYS_CLOCK_IPO:
        MXC_GCR->clkctrl &= ~MXC_F_GCR_CLKCTRL_IPO_EN;
        break;
#ifdef MXC_SYS_CLOCK_ISO // ai85 only    
        
    case MXC_SYS_CLOCK_ISO:
        MXC_GCR->clkctrl &= ~MXC_F_GCR_CLKCTRL_ISO_EN;
        break;
#endif
        
    case MXC_SYS_CLOCK_IBRO:
        MXC_GCR->clkctrl &= ~MXC_F_GCR_CLKCTRL_IBRO_EN;
        break;
        
    case MXC_SYS_CLOCK_EXTCLK:
        // MXC_GCR->clkctrl &= ~MXC_F_GCR_CLKCTRL_EXTCLK_EN;
        break;
        
    case MXC_SYS_CLOCK_INRO:
        // The 80k clock is always enabled
        break;
        
#ifdef MXC_SYS_CLOCK_ERFO // ME17 only    
        
    case MXC_SYS_CLOCK_ERFO:
        MXC_GCR->clkctrl &= ~MXC_F_GCR_CLKCTRL_ERFO_EN;
        break;
#endif
        
    case MXC_SYS_CLOCK_ERTCO:
        MXC_GCR->clkctrl &= ~MXC_F_GCR_CLKCTRL_ERTCO_EN;
        break;
        
    default:
        return E_BAD_PARAM;
    }
    
    return E_NO_ERROR;
}

/* ************************************************************************** */
int MXC_SYS_Clock_Timeout(uint32_t ready)
{
    #ifdef __riscv
        // The current RISC-V implementation is to block until the clock is ready.
        // We do not have access to a system tick in the RV core.
        while(!(MXC_GCR->clkctrl & ready));
        return E_NO_ERROR;
    #else
        // Start timeout, wait for ready
        MXC_DelayAsync(MXC_SYS_CLOCK_TIMEOUT, NULL);
        
        do {
            if (MXC_GCR->clkctrl & ready) {
                MXC_DelayAbort();
                return E_NO_ERROR;
            }
        }
        while (MXC_DelayCheck() == E_BUSY);
        
        return E_TIME_OUT;
    #endif // __riscv
}

/* ************************************************************************** */
int MXC_SYS_Clock_Select(mxc_sys_system_clock_t clock)
{
    uint32_t current_clock;
    
    // Save the current system clock
    current_clock = MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_SYSCLK_SEL;
    
    switch (clock) {
    case MXC_SYS_CLOCK_IPO:
    
        // Enable IPO clock
        if (!(MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_IPO_EN)) {
        
            MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_IPO_EN;
            
            // Check if IPO clock is ready
            if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_IPO_RDY) != E_NO_ERROR) {
                return E_TIME_OUT;
            }
        }
        
        // Set IPO clock as System Clock
        MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, MXC_S_GCR_CLKCTRL_SYSCLK_SEL_IPO);
        
        break;
        
#ifdef MXC_SYS_CLOCK_ISO // AI85 only 
        
    case MXC_SYS_CLOCK_ISO:
    
        // Enable ISO clock
        if (!(MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_ISO_EN)) {
            MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_ISO_EN;
            
            // Check if ISO clock is ready
            if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_ISO_RDY) != E_NO_ERROR) {
                return E_TIME_OUT;
            }
        }
        
        // Set ISO clock as System Clock
        MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, MXC_S_GCR_CLKCTRL_SYSCLK_SEL_ISO);
        
        break;
#endif
        
    case MXC_SYS_CLOCK_IBRO:
    
        // Enable IBRO clock
        if (!(MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_IBRO_EN)) {
            MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_IBRO_EN;
            
            // Check if IBRO clock is ready
            if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_IBRO_RDY) != E_NO_ERROR) {
                return E_TIME_OUT;
            }
        }
        
        // Set IBRO clock as System Clock
        MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, MXC_S_GCR_CLKCTRL_SYSCLK_SEL_IBRO);
        
        break;
        
    case MXC_SYS_CLOCK_EXTCLK:
        // Enable HIRC clock
        // if(!(MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_EXTCLK_EN)) {
        //     MXC_GCR->clkctrl |=MXC_F_GCR_CLKCTRL_EXTCLK_EN;
        
        //     // Check if HIRC clock is ready
        //     if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_EXTCLK_RDY) != E_NO_ERROR) {
        //         return E_TIME_OUT;
        //     }
        // }
        
        // Set HIRC clock as System Clock
        // MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, MXC_S_GCR_CLKCTRL_SYSCLK_SEL_EXTCLK);
        
        break;
        
#ifdef MXC_SYS_CLOCK_ERFO // ME17 only 
        
    case MXC_SYS_CLOCK_ERFO:
    
        // Enable ERFO clock
        if (!(MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_ERFO_EN)) {
            MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_ERFO_EN;
            
            // Check if ERFO clock is ready
            if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_ERFO_RDY) != E_NO_ERROR) {
                return E_TIME_OUT;
            }
        }
        
        // Set ERFO clock as System Clock
        MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, MXC_S_GCR_CLKCTRL_SYSCLK_SEL_ERFO);
        
        break;
#endif
        
    case MXC_SYS_CLOCK_INRO:
        // Set INRO clock as System Clock
        MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, MXC_S_GCR_CLKCTRL_SYSCLK_SEL_INRO);
        
        break;
        
    case MXC_SYS_CLOCK_ERTCO:
    
        // Enable ERTCO clock
        if (!(MXC_GCR->clkctrl & MXC_F_GCR_CLKCTRL_ERTCO_EN)) {
            MXC_GCR->clkctrl |= MXC_F_GCR_CLKCTRL_ERTCO_EN;
            
            // Check if ERTCO clock is ready
            if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_ERTCO_RDY) != E_NO_ERROR) {
                return E_TIME_OUT;
            }
        }
        
        // Set ERTCO clock as System Clock
        MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, MXC_S_GCR_CLKCTRL_SYSCLK_SEL_ERTCO);
        
        break;
        
    default:
        return E_BAD_PARAM;
    }
    
    // Wait for system clock to be ready
    if (MXC_SYS_Clock_Timeout(MXC_F_GCR_CLKCTRL_SYSCLK_RDY) != E_NO_ERROR) {
    
        // Restore the old system clock if timeout
        MXC_SETFIELD(MXC_GCR->clkctrl, MXC_F_GCR_CLKCTRL_SYSCLK_SEL, current_clock);
        
        return E_TIME_OUT;
    }
    
    // Update the system core clock
    SystemCoreClockUpdate();
    
    return E_NO_ERROR;
}

/* ************************************************************************** */
void MXC_SYS_Reset_Periph(mxc_sys_reset_t reset)
{
    /* The mxc_sys_reset_t enum uses enum values that are the offset by 32 and 64 for the rst register. */
    if (reset > 63) {
        reset -= 64;
        MXC_LPGCR->rst = (0x1 << reset);
    }
    else if (reset > 31) {
        reset -= 32;
        MXC_GCR->rst1  = (0x1 << reset);
    }
    else {
        MXC_GCR->rst0  = (0x1 << reset);
    }
}
/**@} end of mxc_sys */
