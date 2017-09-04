/*
 * File Name:   common.c
 *
 * Author:      John Boyd
 * Date:        Sep 4, 2017
 */

#include "common.h"

void delayMillisecond(unsigned int delay)
{
    while(delay--)
    {
        // Delay one millisecond
        __delay_cycles(UCS_MCLK_FREQUENCY_IN_KHZ);
    }

    return;
}
