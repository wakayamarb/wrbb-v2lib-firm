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
	mrb_define_global_const(mrb, "C_BLACK",   mrb_fixnum_value(0x0000));
	mrb_define_global_const(mrb, "C_BROWN",   mrb_fixnum_value(0xcb43));
	mrb_define_global_const(mrb, "C_RED",     mrb_fixnum_value(0xf800));
	mrb_define_global_const(mrb, "C_ORANGE",  mrb_fixnum_value(0xfd20));
	mrb_define_global_const(mrb, "C_YELLOW",  mrb_fixnum_value(0xffe0));
	mrb_define_global_const(mrb, "C_LIME",    mrb_fixnum_value(0x07e0));
	mrb_define_global_const(mrb, "C_BLUE",    mrb_fixnum_value(0x001f));
	mrb_define_global_const(mrb, "C_VIOLET",  mrb_fixnum_value(0x901a));
	mrb_define_global_const(mrb, "C_GRAY",    mrb_fixnum_value(0xa554));
	mrb_define_global_const(mrb, "C_WHITE",   mrb_fixnum_value(0xffff));
	mrb_define_global_const(mrb, "C_CYAN",    mrb_fixnum_value(0x07ff));
	mrb_define_global_const(mrb, "C_MAGENTA", mrb_fixnum_value(0xf81f));
}
