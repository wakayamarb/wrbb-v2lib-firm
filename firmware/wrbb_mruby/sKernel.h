/*
 * カーネル関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#ifndef _SKERNEL_H_
#define _SKERNEL_H_  1

//**************************************************
// ライブラリを定義します
//**************************************************
void kernel_Init(mrb_state *mrb);

//**************************************************
// WRBB - SAKURAピン番コンバート
//**************************************************
int wrb2sakura(int pin);

#endif // _SKERNEL_H_
