/* --COPYRIGHT--,BSD
 * Copyright (c) 2016, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * --/COPYRIGHT--*/

#include <driverlib.h>
#include "common.h"
#include "hardware/trf79xxa.h"
#include "nfc/nfc_app.h"

// Forward declarations
void configureClocks();

void main(void)
{
    //====Initialize MCU=========================================
#ifdef DISABLE_WDT
    // Stop watchdog timer
    WDT_A_hold(WDT_A_BASE);
#endif
    configureClocks();

    // Configure GPIO pin modes
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN1 + GPIO_PIN2);
    //===========================================================

    //=====Initialize TRF79xxA Module============================
    // Set SPI slave select port high
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);

    // Four millisecond delay between bringing SS high
    // and then set EN high per TRF7970A Datasheet
    delayMillisecond(4);
    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);

    // Wait until TRF system clock started
    delayMillisecond(5);

    // Set up TRF initial settings
    TRF79xxA_initialSettings();
    TRF79xxA_setTrfPowerSetting(TRF79xxA_3V_FULL_POWER);
    //===========================================================
}

void configureClocks()
{
    // Set VCore = 1 for 12MHz clock
    PMM_setVCore(PMM_CORE_LEVEL_1);

    // Set DCO FLL reference = REFO
    // Uses precise reference to stabilize DCO
    UCS_initClockSignal(
            UCS_FLLREF,
            UCS_REFOCLK_SELECT,
            UCS_CLOCK_DIVIDER_1
            );

    // Set FLL ratio, desired MCLK frequency and initialize DCO
    UCS_initFLLSettle(
            UCS_MCLK_FREQUENCY_IN_KHZ,
            UCS_MCLK_FLLREF_RATIO
            );

    //Set ACLK = REFO
    UCS_initClockSignal(
            UCS_ACLK,
            UCS_REFOCLK_SELECT,
            UCS_CLOCK_DIVIDER_1
            );

    // Enable global oscillator fault flag
    SFR_clearInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);
    SFR_enableInterrupt(SFR_OSCILLATOR_FAULT_INTERRUPT);

    // Enable global interrupt
    __bis_SR_register(GIE);

    // Verify MCLK frequency within +/- 1% of target
    volatile uint32_t clockFreq = UCS_getMCLK();
    if(     (clockFreq > (uint32_t) 12120000) ||
            (clockFreq < (uint32_t) 11880000) )
    {
        // ERROR! Your clock settings are incorrect
        while(1);
    }

    return;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=UNMI_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(UNMI_VECTOR)))
#endif
void NMI_ISR(void)
{
    // Oscillator fault flags
    uint16_t status;

    do
    {
        // If it still can't clear the oscillator fault flags after the timeout,
        // trap and wait here.
        status = UCS_clearAllOscFlagsWithTimeout(1000);
    }
    while(status != 0);
}
