/*
 * DC Motor関連
 *
 * Copyright (c) 2017 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>

#include <mruby.h>
//#include <mruby/string.h>

#include "../wrbb.h"

#include "sDCMotor.h"

#define M1_SPEED	4
#define M1_LOGIC0	18
#define M1_LOGIC1	3

#define M2_SPEED 	10
#define M2_LOGIC0	15
#define M2_LOGIC1	14

//**************************************************
// 指定したモータにブレーキを掛けます: Motor.brake
//  Motor.brake([num])
//  num: 0:両方にブレーキ, 1:モータ1にブレーキ, 2:モータ2にブレーキ
//
//  省略時は両方にブレーキを掛けます
//**************************************************
mrb_value mrb_dcMotor_brake(mrb_state *mrb, mrb_value self)
{
int num;

	int n = mrb_get_args(mrb, "|i", &num);

	if (n < 1) {
		num = 0;
	}

	if (num == 0) {
		digitalWrite(M1_LOGIC0, LOW);
		digitalWrite(M1_LOGIC1, LOW);
		digitalWrite(M2_LOGIC0, LOW);
		digitalWrite(M2_LOGIC1, LOW);
	}
	else if (num == 1) {
		digitalWrite(M1_LOGIC0, LOW);
		digitalWrite(M1_LOGIC1, LOW);
	}
	else if (num == 2) {
		digitalWrite(M2_LOGIC0, LOW);
		digitalWrite(M2_LOGIC1, LOW);
	}

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// 指定したモータをフリーにします: Motor.free
//  Motor.free([num])
//  num: 0:両方フリー, 1:モータ1をフリー, 2:モータ2をフリー
//
//  省略時は両方をフリーにします
//**************************************************
mrb_value mrb_dcMotor_free(mrb_state *mrb, mrb_value self)
{
	int num;

	int n = mrb_get_args(mrb, "|i", &num);

	if (n < 1) {
		num = 0;
	}

	if (num == 0) {
		digitalWrite(M1_LOGIC0, HIGH);
		digitalWrite(M1_LOGIC1, HIGH);
		digitalWrite(M2_LOGIC0, HIGH);
		digitalWrite(M2_LOGIC1, HIGH);
	}
	else if (num == 1) {
		digitalWrite(M1_LOGIC0, HIGH);
		digitalWrite(M1_LOGIC1, HIGH);
	}
	else if (num == 2) {
		digitalWrite(M2_LOGIC0, HIGH);
		digitalWrite(M2_LOGIC1, HIGH);
	}

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// 指定したモータをcw(時計回り)にします: Motor.cw
//  Motor.cw([num])
//  num: 0:cw(時計回り), 1:モータ1をcw(時計回り), 2:モータ2をcw(時計回り)
//
//  省略時は両方をcw(時計回り)にします
//**************************************************
mrb_value mrb_dcMotor_cw(mrb_state *mrb, mrb_value self)
{
	int num;

	int n = mrb_get_args(mrb, "|i", &num);

	if (n < 1) {
		num = 0;
	}

	if (num == 0) {
		digitalWrite(M1_LOGIC0, HIGH);
		digitalWrite(M1_LOGIC1, LOW);
		digitalWrite(M2_LOGIC0, HIGH);
		digitalWrite(M2_LOGIC1, LOW);
	}
	else if (num == 1) {
		digitalWrite(M1_LOGIC0, HIGH);
		digitalWrite(M1_LOGIC1, LOW);
	}
	else if (num == 2) {
		digitalWrite(M2_LOGIC0, HIGH);
		digitalWrite(M2_LOGIC1, LOW);
	}

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// 指定したモータをccw(反時計回り)にします: Motor.ccw
//  Motor.ccw([num])
//  num: 0:ccw(反時計回り), 1:モータ1をccw(反時計回り), 2:モータ2をccw(反時計回り)
//
//  省略時は両方をccw(反時計回り)にします
//**************************************************
mrb_value mrb_dcMotor_ccw(mrb_state *mrb, mrb_value self)
{
	int num;

	int n = mrb_get_args(mrb, "|i", &num);

	if (n < 1) {
		num = 0;
	}

	if (num == 0) {
		digitalWrite(M1_LOGIC0, LOW);
		digitalWrite(M1_LOGIC1, HIGH);
		digitalWrite(M2_LOGIC0, LOW);
		digitalWrite(M2_LOGIC1, HIGH);
	}
	else if (num == 1) {
		digitalWrite(M1_LOGIC0, LOW);
		digitalWrite(M1_LOGIC1, HIGH);
	}
	else if (num == 2) {
		digitalWrite(M2_LOGIC0, LOW);
		digitalWrite(M2_LOGIC1, HIGH);
	}

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// 指定したモータの回転速度を指定します: Motor.speed
//  Motor.speed(num, speed)
//  num: 1:モータ1, 2:モータ2
//  speed: 0～255
//**************************************************
mrb_value mrb_dcMotor_speed(mrb_state *mrb, mrb_value self)
{
int num;
int spd;

	mrb_get_args(mrb, "ii", &num, &spd);

	if (num == 1) {
		analogWrite(M1_SPEED, spd);
	}
	else if (num == 2) {
		analogWrite(M2_SPEED, spd);
	}

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// ライブラリを定義します
//**************************************************
void dcMotor_Init(mrb_state *mrb)
{
	pinMode(M1_LOGIC0, OUTPUT);
	pinMode(M1_LOGIC1, OUTPUT);
	pinMode(M2_LOGIC0, OUTPUT);
	pinMode(M2_LOGIC1, OUTPUT);

	struct RClass *dcMotorModule = mrb_define_module(mrb, "Motor");

	mrb_define_module_function(mrb, dcMotorModule, "brake", mrb_dcMotor_brake, MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, dcMotorModule, "free", mrb_dcMotor_free, MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, dcMotorModule, "cw", mrb_dcMotor_cw, MRB_ARGS_OPT(1));		//ClockWise
	mrb_define_module_function(mrb, dcMotorModule, "ccw", mrb_dcMotor_ccw, MRB_ARGS_OPT(1));	//CounterClockWise
	mrb_define_module_function(mrb, dcMotorModule, "speed", mrb_dcMotor_speed, MRB_ARGS_REQ(2));
}
