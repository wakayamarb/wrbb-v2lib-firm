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
#include "rx63n/typedefine.h"

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

void fftbegin();

float mean(float* data, int length);
int mean(int* data, int length);
int16_t mean(int16_t* data, int length);


