/*
 * グローバル変数関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <mruby.h>

#include "../wrbb.h"

//**************************************************
// 定義します
//**************************************************
void global_Init(mrb_state *mrb)
{
	mrb_define_global_const(mrb, "ON", mrb_fixnum_value(1));
	mrb_define_global_const(mrb, "OFF", mrb_fixnum_value(0));
	mrb_define_global_const(mrb, "HIGH", mrb_fixnum_value(1));
	mrb_define_global_const(mrb, "LOW", mrb_fixnum_value(0));
	mrb_define_global_const(mrb, "INPUT", mrb_fixnum_value(0));
	mrb_define_global_const(mrb, "OUTPUT", mrb_fixnum_value(1));
	//mrb_define_global_const(mrb, "INPUT_PULLUP", mrb_fixnum_value(2));
}
