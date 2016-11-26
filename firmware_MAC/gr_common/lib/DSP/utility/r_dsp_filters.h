/*******************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only
* intended for use with Renesas products. No other uses are authorized. This
* software is owned by Renesas Electronics Corporation and is protected under
* all applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT
* LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED.
* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS
* ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR
* ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE
* BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software
* and to discontinue the availability of this software. By using this software,
* you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2012 Renesas Electronics Corporation. All rights reserved.
*******************************************************************************/

/*******************************************************************************
* File Name    : r_dsp_filters.h
* Description  : PUBLIC header file for filter kernels of DSP library
******************************************************************************/
/******************************************************************************
* History      : YYYY.MM.DD  Version  Description
*              : 2012.08.01  2.00     Second version
******************************************************************************/

#ifndef R_DSP_FILTERS_H
#define R_DSP_FILTERS_H


/******************************************************************************
Includes
******************************************************************************/
#include "r_dsp_types.h"


/******************************************************************************
Macro definitions
******************************************************************************/

/******************************************************************************
Typedef definitions
******************************************************************************/
/*---------------------------*/
/* Generic FIR filter        */
/*---------------------------*/
typedef struct
{
    uint32_t taps;     // number of filter taps
    void *   coefs;    // pointer to filter coefficients
    void *   state;    // pointer to filter state data, including the filter’s delay line
                       // and any other implementation-dependent state
    int32_t  scale;    // scaling parameter for fixed-point data management
    uint32_t options;  // options that specify rounding, saturation, or other behaviors
} r_dsp_firfilter_t;

/*---------------------------*/
/* IIR Biquad filter         */
/*---------------------------*/
typedef enum {
    DEFAULT_BIQUAD_FORM  = 0,
    DIRECT_BIQUAD_FORM_I = 1,
    DIRECT_BIQUAD_FORM_II
  // other forms can be added here in the future.
}r_dsp_iirbiquad_form_t;

typedef struct
{
    uint32_t stages;     // number of biquad stages
    void *   coefs;      // pointer to filter coefficients
    void *   state;      // pointer to filter’s internal state (delay line)
    int32_t  scale;      // scaling parameter for fixed-point data management
    uint32_t qint;       // number of integer bit (0 or 1) for coefficients
    uint32_t options;    // options that specify rounding, saturation, or other behaviors
    uint32_t form;  //which biquad form to use
} r_dsp_iirbiquad_t;

/*---------------------------*/
/* Leaky LMS Adaptive filter */
/*---------------------------*/
typedef struct
{
    uint32_t taps;     // number of FIR taps
    void *   coefs;    // pointer to filter coefficients
    void *   state;    // pointer to filter’s internal state (delay line)
    int32_t  scale;    // scaling parameter for fixed-point data management
    uint32_t options;  // options that specify rounding, saturation, or other behaviors
   union
   {
       uint32_t ustep;
       float fstep;
   } step; // step size, see algorithmic description
   union
   {
       uint32_t uleak;
       float fleak;
   } leak; // leak, see algorithmic description
} r_dsp_lmsleaky_t;

/*---------------------------*/
/* Lattice FIR filter        */
/*---------------------------*/
typedef struct
{
    uint32_t stages;  // number of lattice sections
    void *   coefs;   // pointer to filter coefficients
    void *   state;   // pointer to filter’s internal state (delay line)
    int32_t  scale;   // scaling parameter for fixed-point data management
    uint32_t options; // options that specify rounding, saturation, or other behaviors
} r_dsp_firlattice_t;

/*---------------------------*/
/* Lattice IIR filter        */
/*---------------------------*/
typedef struct
{
    uint32_t stages;  // number of lattice sections
    void *   coefs;   // pointer to filter coefficients
    void *   state;   // pointer to filter’s internal state (delay line)
    int32_t  scale;   // scaling parameter for fixed-point data management
    uint32_t options; // options that specify rounding, saturation, or other behaviors
} r_dsp_iirlattice_t;

/*---------------------------*/
/* Single-pole IIR filter    */
/*---------------------------*/
typedef struct
{
    void     *coefs;   // pointer to filter coefficient
    void     *state;   // pointer to feedback state
    int32_t  scale;    // scaling parameter for fixed-point data management
    uint32_t  options; // options that specify rounding, saturation, or other behaviors
} r_dsp_iirsinglepole_t;


/******************************************************************************
Filter API Function prototypes
******************************************************************************/
/*---------------------------*/
/* Generic FIR filter        */
/*---------------------------*/
/* Real input: f32; Real output: f32 */
r_dsp_status_t R_DSP_FIR_Init_f32f32 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_f32f32 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i32; Real output: i32 */
r_dsp_status_t R_DSP_FIR_Init_i32i32 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_i32i32 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i32 */
r_dsp_status_t R_DSP_FIR_Init_i16i32 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_i16i32 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i16 */
r_dsp_status_t R_DSP_FIR_Init_i16i16 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_i16i16 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: cf32; Complex output: cf32 */
r_dsp_status_t R_DSP_FIR_Init_cf32cf32 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_cf32cf32 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci32; Complex output: ci32 */
r_dsp_status_t R_DSP_FIR_Init_ci32ci32 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_ci32ci32 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci32 */
r_dsp_status_t R_DSP_FIR_Init_ci16ci32 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_ci16ci32 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci16 */
r_dsp_status_t R_DSP_FIR_Init_ci16ci16 (
    r_dsp_firfilter_t * handle
);

r_dsp_status_t R_DSP_FIR_ci16ci16 (
    const r_dsp_firfilter_t * handle,
    const vector_t * input,
    vector_t * output
);


/*---------------------------*/
/* IIR Biquad filter         */
/*---------------------------*/
/* Real input: f32; Real output: f32 */
int32_t R_DSP_IIRBiquad_StateSize_f32f32 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_f32f32 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_f32f32 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i32; Real output: i32 */
int32_t R_DSP_IIRBiquad_StateSize_i32i32 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_i32i32 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_i32i32 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i32 */
int32_t R_DSP_IIRBiquad_StateSize_i16i32 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_i16i32 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_i16i32 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i16 */
int32_t R_DSP_IIRBiquad_StateSize_i16i16 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_i16i16 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_i16i16 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: cf32; Complex output: cf32 */
int32_t R_DSP_IIRBiquad_StateSize_cf32cf32 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_cf32cf32 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_cf32cf32 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci32; Complex output: ci32 */
int32_t R_DSP_IIRBiquad_StateSize_ci32ci32 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_ci32ci32 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_ci32ci32 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci32 */
int32_t R_DSP_IIRBiquad_StateSize_ci16ci32 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_ci16ci32 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_ci16ci32 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci16 */
int32_t R_DSP_IIRBiquad_StateSize_ci16ci16 (
    const r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_Init_ci16ci16 (
    r_dsp_iirbiquad_t * handle
);

r_dsp_status_t R_DSP_IIRBiquad_ci16ci16 (
    const r_dsp_iirbiquad_t * handle,
    const vector_t * input,
    vector_t * output
);


/*---------------------------*/
/* Leaky LMS Adaptive filter */
/*---------------------------*/
/* Real input: f32; Real output: f32 */
r_dsp_status_t R_DSP_LMSLeaky_Init_f32f32 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_f32f32 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);

/* Real input: i32; Real output: i32 */
r_dsp_status_t R_DSP_LMSLeaky_Init_i32i32 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_i32i32 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);

/* Real input: i16; Real output: i32 */
r_dsp_status_t R_DSP_LMSLeaky_Init_i16i32 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_i16i32 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);

/* Real input: i16; Real output: i16 */
r_dsp_status_t R_DSP_LMSLeaky_Init_i16i16 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_i16i16 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);

/* Complex input: cf32; Complex output: cf32 */
r_dsp_status_t R_DSP_LMSLeaky_Init_cf32cf32 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_cf32cf32 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);

/* Complex input: ci32; Complex output: ci32 */
r_dsp_status_t R_DSP_LMSLeaky_Init_ci32ci32 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_ci32ci32 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);

/* Complex input: ci16; Complex output: ci32 */
r_dsp_status_t R_DSP_LMSLeaky_Init_ci16ci32 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_ci16ci32 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);

/* Complex input: ci16; Complex output: ci16 */
r_dsp_status_t R_DSP_LMSLeaky_Init_ci16ci16 (
    r_dsp_lmsleaky_t * handle
);

r_dsp_status_t R_DSP_LMSLeaky_ci16ci16 (
    const r_dsp_lmsleaky_t * handle,
    const vector_t * input,
    const vector_t * refer,
    vector_t * output,
    vector_t * residual
);


/*---------------------------*/
/* Lattice FIR filter        */
/*---------------------------*/
/* Real input: f32; Real output: f32 */
int32_t R_DSP_FIRLattice_StateSize_f32f32 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_f32f32 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_f32f32 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i32; Real output: i32 */
int32_t R_DSP_FIRLattice_StateSize_i32i32 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_i32i32 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_i32i32 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i32 */
int32_t R_DSP_FIRLattice_StateSize_i16i32 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_i16i32 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_i16i32 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i16 */
int32_t R_DSP_FIRLattice_StateSize_i16i16 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_i16i16 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_i16i16 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: cf32; Complex output: cf32 */
int32_t R_DSP_FIRLattice_StateSize_cf32cf32 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_cf32cf32 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_cf32cf32 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci32; Complex output: ci32 */
int32_t R_DSP_FIRLattice_StateSize_ci32ci32 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_ci32ci32 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_ci32ci32 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci32 */
int32_t R_DSP_FIRLattice_StateSize_ci16ci32 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_ci16ci32 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_ci16ci32 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci16 */
int32_t R_DSP_FIRLattice_StateSize_ci16ci16 (
    const r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_Init_ci16ci16 (
    r_dsp_firlattice_t * handle
);

r_dsp_status_t R_DSP_FIRLattice_ci16ci16 (
    const r_dsp_firlattice_t * handle,
    const vector_t * input,
    vector_t * output
);


/*---------------------------*/
/* Lattice IIR filter        */
/*---------------------------*/
/* Real input: f32; Real output: f32 */
int32_t R_DSP_IIRLattice_StateSize_f32f32 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_f32f32 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_f32f32 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i32; Real output: i32 */
int32_t R_DSP_IIRLattice_StateSize_i32i32 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_i32i32 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_i32i32 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i32 */
int32_t R_DSP_IIRLattice_StateSize_i16i32 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_i16i32 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_i16i32 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i16 */
int32_t R_DSP_IIRLattice_StateSize_i16i16 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_i16i16 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_i16i16 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: cf32; Complex output: cf32 */
int32_t R_DSP_IIRLattice_StateSize_cf32cf32 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_cf32cf32 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_cf32cf32 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci32; Complex output: ci32 */
int32_t R_DSP_IIRLattice_StateSize_ci32ci32 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_ci32ci32 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_ci32ci32 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci32 */
int32_t R_DSP_IIRLattice_StateSize_ci16ci32 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_ci16ci32 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_ci16ci32 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Complex input: ci16; Complex output: ci16 */
int32_t R_DSP_IIRLattice_StateSize_ci16ci16 (
    const r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_Init_ci16ci16 (
    r_dsp_iirlattice_t * handle
);

r_dsp_status_t R_DSP_IIRLattice_ci16ci16 (
    const r_dsp_iirlattice_t * handle,
    const vector_t * input,
    vector_t * output
);


/*---------------------------*/
/* Single-pole IIR filter    */
/*---------------------------*/
/* Real input: f32; Real output: f32 */
r_dsp_status_t R_DSP_IIRSinglePole_f32f32 (
    const r_dsp_iirsinglepole_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i32; Real output: i32 */
r_dsp_status_t R_DSP_IIRSinglePole_i32i32 (
    const r_dsp_iirsinglepole_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i32 */
r_dsp_status_t R_DSP_IIRSinglePole_i16i32 (
    const r_dsp_iirsinglepole_t * handle,
    const vector_t * input,
    vector_t * output
);

/* Real input: i16; Real output: i16 */
r_dsp_status_t R_DSP_IIRSinglePole_i16i16 (
    const r_dsp_iirsinglepole_t * handle,
    const vector_t * input,
    vector_t * output
);

#endif
/* End of file */


