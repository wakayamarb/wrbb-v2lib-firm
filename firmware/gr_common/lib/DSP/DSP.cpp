/***************************************************************************
 *
 * PURPOSE
 *   RX63N framework basic DSP function.
 *
 * TARGET DEVICE
 *   RX63N
 *
 * AUTHOR
 *   Renesas Electronics Corp.
 *
 ***************************************************************************
 * Copyright (C) 2014 Renesas Electronics. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * See file LICENSE.txt for further informations on licensing terms.
 ***************************************************************************/
/**
 * Modified dd mm yyyy : name : description
 */
/***************************************************************************/
/*    Include Header Files                                                 */
/***************************************************************************/

#include "utility/r_dsp_types.h"
#include "rx63n/iodefine.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "utility/r_dsp_statistical.h"
#include "utility/r_dsp_transform.h"

#ifdef __cplusplus
}
#endif

/***************************************************************************/
/*    Macro Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Type  Definitions                                                    */
/***************************************************************************/

/***************************************************************************/
/*    Function prototypes                                                  */
/***************************************************************************/

/***************************************************************************/
/*    Global Variables                                                     */
/***************************************************************************/
#define FFT_POINTS 256
/*
static r_dsp_fft_t h = {FFT_POINTS, 0 };    // default options set
static size_t ntwb;                         // bytes for twiddle array
static size_t nbrb;                         // bytes for bit-reverse table
static size_t nwkb;                         // bytes for work area
static vector_t vtime = {FFT_POINTS, (void *)g_adc_buf1};
static vector_t vfreq = {FFT_POINTS/2, (void *)g_fft_output};

static int16_t fft_twiddles[FFT_POINTS+FFT_POINTS/2];
static int32_t fft_bitrev[56];                   // case of 256data


void fftbegin(){
    r_dsp_status_t status = R_DSP_STATUS_OK;

    status = R_DSP_FFT_BufSize_i16ci16(&h, &ntwb, &nbrb, &nwkb);
    vtime.data = (void *)g_adc_buf1;

    h.n = FFT_POINTS;
    h.work = NULL;
    h.options = R_DSP_FFT_SCALE_DEFAULT | R_DSP_FFT_BIT_REVERSAL_DEFAULT;
    h.bitrev = (void *)fft_bitrev;
    // set R_DSP_FFT_OPT_NO_BITREV like h.options |= R_DSP_FFT_OPT_NO_BITREV, if you don't need bit-reverse
    h.twiddles = (void *)fft_twiddles;
    h.window = NULL;    // always NULL in Complex FFT
    status = R_DSP_FFT_Init_ci16ci16(&h);



}
*/

float mean(float* data, int length){
    vector_t vector;
    float mean;

    vector.n = (uint32_t)length;
    vector.data = data;

    R_DSP_Mean_f32(&vector, &mean);
    return mean;
}

int mean(int* data, int length){
    vector_t vector;
    int32_t mean;

    vector.n = (uint32_t)length;
    vector.data = data;

    R_DSP_Mean_i32(&vector, &mean);
    return mean;
}

int16_t mean(int16_t* data, int length){
    vector_t vector;
    int16_t mean;

    vector.n = (uint32_t)length;
    vector.data = data;

    R_DSP_Mean_i16(&vector, &mean);
    return mean;
}


