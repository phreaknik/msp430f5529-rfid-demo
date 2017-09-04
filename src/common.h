/*
 * File Name:   common.h
 *
 * Author:      John Boyd
 * Date:        Sep 4, 2017
 */

#ifndef COMMON_H_
#define COMMON_H_

//=====Clock Settings============================================
#define UCS_MCLK_FREQUENCY_IN_KHZ 12000
#define UCS_MCLK_FLLREF_RATIO 366 // (MCLK_FREQ / FLL_REF_FREQ)
//===============================================================

//=====Common System API=========================================
void delayMillisecond(unsigned int delay);
//===============================================================

#endif
