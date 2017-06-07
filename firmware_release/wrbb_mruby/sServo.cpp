/*
 * サーボ・ハード関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <Servo.h>

#include <mruby.h>
//#include <mruby/string.h>
//#include <mruby/variable.h>

#include "sExec.h"
#include "../wrbb.h"
#include "sKernel.h"

#define ATTACH_MAX	20

//newはシステム起動後、1回しかさせないために、ここで初期化している。
Servo *servo[ATTACH_MAX] ={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
							0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

//**************************************************
// サーボ出力を任意のピンに割り当てます: Servo.attach
// Servo.attach(ch, pin[,min,max])
//	ch: サーボのチャネル 0～11まで指定できます
//  pin: 割り当てるピン番号
//  min: サーボの角度が0度のときのパルス幅(マイクロ秒)。デフォルトは544
//  max: サーボの角度が180度のときのパルス幅(マイクロ秒)。デフォルトは2400
//**************************************************
mrb_value mrb_servo_attach(mrb_state *mrb, mrb_value self)
{
int ch;
int pin;
int min;
int max;

	int n = mrb_get_args(mrb, "ii|ii", &ch, &pin, &min, &max);

	if(pin >= 20){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	//nが3のときは、maxが引数に無いので、
	if (n == 3){
		max = 2400;
	}

	if (ch < 0 || ch >= ATTACH_MAX){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	if ((int)servo[ch] != 0){
		servo[ch]->detach();
		//delete servo[ch];
		//servo[ch] = 0;
	}
	else{
		servo[ch] = new Servo();
	}

	if (n >= 3){
		servo[ch]->attach(pin, min, max);
	}
	else{
		servo[ch]->attach(pin);
	}

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// サーボの角度をセットします: Servo.write
// Servo.write(ch,angle)
//	ch: サーボのチャネル 0～11まで指定できます
//  angle: 角度 0～180
//**************************************************
mrb_value mrb_servo_write(mrb_state *mrb, mrb_value self)
{
	int ch;
	int angle;

	mrb_get_args(mrb, "ii", &ch, &angle);

	if (ch < 0 || ch >= ATTACH_MAX){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	if ((int)servo[ch] == 0){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	DEBUG_PRINT("Servo", angle);

	servo[ch]->write(angle);

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// サーボモータにus単位で角度を指定する: Servo.us
// Servo.us(ch,us)
//	ch: サーボのチャネル 0～11まで指定できます
//  us: 出力したいパルスの幅 1～19999, 0で出力 OFF
//   サーボモータに与えられるパルスは20ms周期で、1周期中のHighの時間を直接指定する。
//   実質的にPWM出力。連続回転タイプのサーボでは、回転のスピードが設定することができる。
//**************************************************
mrb_value mrb_servo_us(mrb_state *mrb, mrb_value self)
{
	int ch;
	int us;

	mrb_get_args(mrb, "ii", &ch, &us);

	if (ch < 0 || ch >= ATTACH_MAX){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	if ((int)servo[ch] == 0){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	servo[ch]->writeMicroseconds(us);

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// 最後に設定された角度を読み出す: Servo.read
// Servo.read(ch)
//	ch: サーボのチャネル 0～11まで指定できます
// 戻り値
//  マイクロ秒単位 us(ch) で与えた値は読みとれない
//**************************************************
mrb_value mrb_servo_read(mrb_state *mrb, mrb_value self)
{
	int ch;
	int ret = 0;

	mrb_get_args(mrb, "i", &ch);

	if (ch < 0 || ch >= ATTACH_MAX){
		return mrb_fixnum_value(ret);
	}

	if ((int)servo[ch] == 0){
		return mrb_fixnum_value(ret);
	}

	ret = servo[ch]->read();

	return mrb_fixnum_value(ret);
}

mrb_value Is_attached(mrb_state *mrb, mrb_value self, int mode)
{
	int ch;
	int ret = 0;

	mrb_get_args(mrb, "i", &ch);

	if (ch < 0 || ch >= ATTACH_MAX){
		return (mode == 0?mrb_fixnum_value(0):mrb_bool_value(FALSE));
	}

	if ((int)servo[ch] == 0){
		return (mode == 0?mrb_fixnum_value(0):mrb_bool_value(FALSE));
	}

	if (servo[ch]->attached()){
		ret = 1;
		return (mode == 0?mrb_fixnum_value(1):mrb_bool_value(TRUE));
	}

	return (mode == 0?mrb_fixnum_value(ret):mrb_bool_value(ret == 1));
}

//**************************************************
// ピンにサーボが割り当てられているかを確認する: Servo.attached
// Servo.attached(ch)
//	ch: サーボのチャネル 0～11まで指定できます
// 戻り値
//  1: 割り当てられている
//  0: 割り当てはない
//**************************************************
mrb_value mrb_servo_attached(mrb_state *mrb, mrb_value self)
{
	return Is_attached(mrb, self, 0);
}
//**************************************************
// ピンにサーボが割り当てられているかを確認する: Servo.attached?
// Servo.attached?(ch)
//	ch: サーボのチャネル 0～11まで指定できます
// 戻り値
//  true: 割り当てられている
//  false: 割り当てはない
//**************************************************
mrb_value mrb_servo_attached_q(mrb_state *mrb, mrb_value self)
{
	return Is_attached(mrb, self, 1);
}

//**************************************************
// サーボの動作を止め、割り込みを禁止する: Servo.detach
// Servo.detach(ch)
//	ch: サーボのチャネル 0～9まで指定できます
//**************************************************
mrb_value mrb_servo_detach(mrb_state *mrb, mrb_value self)
{
	int ch;

	mrb_get_args(mrb, "i", &ch);

	if (ch < 0 || ch >= ATTACH_MAX){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	if ((int)servo[ch] == 0){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	servo[ch]->detach();
	//delete servo[ch];
	//servo[ch] = 0;

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// ライブラリを定義します
//**************************************************
void servo_Init(mrb_state *mrb)
{
	//for (int i = 0; i < ATTACH_MAX; i++){
	//	servo[i] = 0;
	//}

	struct RClass *servoModule = mrb_define_module(mrb, "Servo");

	mrb_define_module_function(mrb, servoModule, "attach", mrb_servo_attach, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(2));
	mrb_define_module_function(mrb, servoModule, "write", mrb_servo_write, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, servoModule, "us", mrb_servo_us, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, servoModule, "read", mrb_servo_read, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, servoModule, "attached", mrb_servo_attached, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, servoModule, "attached?", mrb_servo_attached_q, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, servoModule, "detach", mrb_servo_detach, MRB_ARGS_REQ(1));
}
