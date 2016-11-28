/*
 * 呼び出し実行モジュールプログラム
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
 
#ifndef _SEXEC_H_
#define _SECEC_H_  1

#include <mruby.h>

#if defined __cplusplus
extern "C" {
#endif


#if defined __cplusplus
}
#endif

//**************************************************
//  mrubyを実行します
//**************************************************
bool RubyRun( void );

//**************************************************
//  エラーメッセージ
//**************************************************
void Serial_print_error(mrb_state *mrb, mrb_value obj);


//**************************************************
// ピンモードを入力に初期化します
//**************************************************
void pinModeInit();

#endif // _SEXEC_H_