/*
 * MP3関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#ifndef _SMP3_H_
#define _SMP3_H_  1

#define MP3_CLASS	"MP3"

//**************************************************
// ライブラリを定義します
//**************************************************
int mp3_Init(mrb_state *mrb);

int mp3_Init(mrb_state *mrb,int pausePin, int stopPin);

#endif // _SMP3_H_
