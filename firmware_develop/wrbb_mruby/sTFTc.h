/*
 *  Adafruit TFT 2.8'' Touchpanel Capacitive Shield関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */

#include <Arduino.h>
#include <util.h>

#include <mruby.h>
#include "mruby/string.h"	// RSTRING_PTR(value);

#include "../wrbb.h"

#include <SD.h>			// drawbitmap()にて、File構造体を利用しているため追加


#include <Adafruit_GFX.h>    // Core graphics library
#include <SPI.h>       // this is needed for display
#include <Adafruit_ILI9341.h>
#include <Wire.h>      // this is needed for FT6206
#include <Adafruit_FT6206.h>

// 使いたい追加フォントのコメントを外す
//#include <Fonts/FreeMono9pt7b.h>
//#include <Fonts/FreeMono12pt7b.h>
//#include <Fonts/FreeMono18pt7b.h>
//#include <Fonts/FreeMono24pt7b.h>
//#include <Fonts/FreeMonoBold9pt7b.h>
//#include <Fonts/FreeMonoBold12pt7b.h>
//#include <Fonts/FreeMonoBold18pt7b.h>
//#include <Fonts/FreeMonoBold24pt7b.h>
//#include <Fonts/FreeMonoBoldOblique9pt7b.h>
//#include <Fonts/FreeMonoBoldOblique12pt7b.h>
//#include <Fonts/FreeMonoBoldOblique18pt7b.h>
//#include <Fonts/FreeMonoBoldOblique24pt7b.h>
//#include <Fonts/FreeMonoOblique9pt7b.h>
//#include <Fonts/FreeMonoOblique12pt7b.h>
//#include <Fonts/FreeMonoOblique18pt7b.h>
//#include <Fonts/FreeMonoOblique24pt7b.h>
//#include <Fonts/FreeSans9pt7b.h>
//#include <Fonts/FreeSans12pt7b.h>
//#include <Fonts/FreeSans18pt7b.h>
//#include <Fonts/FreeSans24pt7b.h>
//#include <Fonts/FreeSansBold9pt7b.h>
//#include <Fonts/FreeSansBold12pt7b.h>
//#include <Fonts/FreeSansBold18pt7b.h>
//#include <Fonts/FreeSansBold24pt7b.h>
//#include <Fonts/FreeSansBoldOblique9pt7b.h>
//#include <Fonts/FreeSansBoldOblique12pt7b.h>
//#include <Fonts/FreeSansBoldOblique18pt7b.h>
//#include <Fonts/FreeSansBoldOblique24pt7b.h>
//#include <Fonts/FreeSansOblique9pt7b.h>
//#include <Fonts/FreeSansOblique12pt7b.h>
//#include <Fonts/FreeSansOblique18pt7b.h>
//#include <Fonts/FreeSansOblique24pt7b.h>
//#include <Fonts/FreeSerif9pt7b.h>
//#include <Fonts/FreeSerif12pt7b.h>
//#include <Fonts/FreeSerif18pt7b.h>
//#include <Fonts/FreeSerif24pt7b.h>
//#include <Fonts/FreeSerifBold9pt7b.h>
//#include <Fonts/FreeSerifBold12pt7b.h>
//#include <Fonts/FreeSerifBold18pt7b.h>
//#include <Fonts/FreeSerifBold24pt7b.h>
//#include <Fonts/FreeSerifBoldItalic9pt7b.h>
//#include <Fonts/FreeSerifBoldItalic12pt7b.h>
//#include <Fonts/FreeSerifBoldItalic18pt7b.h>
//#include <Fonts/FreeSerifBoldItalic24pt7b.h>
//#include <Fonts/FreeSerifItalic9pt7b.h>
//#include <Fonts/FreeSerifItalic12pt7b.h>
//#include <Fonts/FreeSerifItalic18pt7b.h>
//#include <Fonts/FreeSerifItalic24pt7b.h>

//**************************************************
// グラフィックのデバッグ時に定義します。
//**************************************************
//#define TFTC_DEBUG

//**************************************************
// 色を定義します
//**************************************************
#define C_BLACK		0x0000	// 0: BLACK
#define C_BROWN		0xcb43  // 1: Chocolate
#define C_RED		0xf800  // 2: Red
#define C_ORANGE	0xfd20  // 3: Orange
#define C_YELLOW	0xffe0  // 4: Yellow
#define C_LIME		0x07e0  // 5: Lime
#define C_BLUE		0x001f  // 6: Blue
#define C_VIOLET	0x901a  // 7: DarkViolet
#define C_GRAY		0xa554  // 8: DarkGray
#define C_WHITE		0xffff  // 9: White
#define C_CYAN		0x07ff  // 10:CYAN
#define C_MAGENTA	0xf81f  // 11:MAGENTA





//**************************************************
// ライブラリを定義します
//**************************************************
int TFTc_Init(mrb_state *mrb);
int TFTc_Clear(mrb_state *mrb);

