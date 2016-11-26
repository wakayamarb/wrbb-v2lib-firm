/***************************************************************************//**
 Copyright   : (c) AND Technology Research Ltd, 2014
 Address     : 4 Forest Drive, Theydon Bois, Essex, CM16 7EY
 Tel         : +44 (0) 1992 81 4655
 Fax         : +44 (0) 1992 81 3362
 Email       : ed.king@andtr.com
 Website     : www.andtr.com

 Project     : <PPP>
 Module      : <MMM>
 File        : rx63n_stdio.h
 Author      : E King
 Start Date  : 11 Mar 2014
 Description :

 ******************************************************************************/

#ifndef RX63N_STDIO_H_
#define RX63N_STDIO_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
/* TYPE DEFINITIONS */

// copied from C:\PROGRA~2\Renesas\Hew\Tools\Renesas\RX\1_2_1/include/sys/yvals.h
#define _FD_TYPE   signed char
typedef unsigned short _Wchart;
#define _MBMAX      8   /* MB_LEN_MAX */

// copied from C:\PROGRA~2\Renesas\Hew\Tools\Renesas\RX\1_2_1/include/stdio.h
#ifndef _MBSTATET
#define _MBSTATET
typedef struct _Mbstatet
{   /* state of a multibyte translation */
unsigned long _Wchar;
unsigned short _Byte, _State;
} _Mbstatet;
#endif /* _MBSTATET */

// DEFINITIONS ****************************************************************/

#ifdef __RX
#pragma unpack
#endif /* __RX */
struct _Dnk_filet
    {   /* file control information */
    FILE * fp;
    unsigned short _Mode;
    unsigned char _Idx;
    _FD_TYPE _Handle;

    unsigned char *_Buf, *_Bend, *_Next;
    unsigned char *_Rend, *_Wend, *_Rback;

    _Wchart *_WRback, _WBack[2];
    unsigned char *_Rsave, *_WRend, *_WWend;

    _Mbstatet _Wstate;
    char *_Tmpnam;
    unsigned char _Back[_MBMAX], _Cbuf;
    };
#ifdef __RX
#pragma packoption
#endif /* __RX */

 #ifndef _FILET
  #define _FILET
typedef struct _Dnk_filet _Filet;
 #endif /* _FILET */

typedef _Filet FILE_USB;


// DECLARATIONS ***************************************************************/

FILE_USB * stdin_usb;
FILE_USB * stdout_usb;
FILE_USB * stderr_usb;

#ifdef __cplusplus
}
#endif

#endif // RX63N_STDIO_H_
