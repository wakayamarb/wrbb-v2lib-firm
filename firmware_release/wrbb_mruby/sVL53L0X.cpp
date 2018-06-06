/*
* VL53L0X関連
*
* Copyright (c) 2018 Wakayama.rb Ruby Board developers
*
* This software is released under the MIT License.
* https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
*
*/
#include <Arduino.h>
#include <Wire.h>
#include <VL53L0X.h>

#include <mruby.h>
//#include <mruby/string.h>

#include "../wrbb.h"

#include "sVL53L0X.h"

VL53L0X sensor;

extern TwoWire *WireVL53L0X;		//0:Wire, 1:Wire1, 2:Wire2, 3:Wire3 4:Wire4 5:Wire5

//**************************************************
// LV53L0Xを初期化します: VL53L0X.init
//  VL53L0X.init(I2Cport[,timeout])
//  I2Cport: I2Cの番号(0～5)
//　timeout: タイムアウト時間 ms (省略時は500ms)
//
// 戻り値
//  true: 初期化成功
//  false: 初期化失敗
//**************************************************
mrb_value mrb_lv53l0x_init(mrb_state *mrb, mrb_value self)
{
	int wirePort, ms;

	int n = mrb_get_args(mrb, "i|i", &wirePort, &ms);

	if (n < 2) {
		ms = 500;
	}

	//0:Wire, 1:Wire1, 2:Wire2, 3:Wire3 4:Wire4 5:Wire5
	switch (wirePort) {
	case 0:
		//ソフトI2Cの場合は18PINと19PINをOUTPUTにする
		pinMode(RB_PIN18, OUTPUT);
		pinMode(RB_PIN19, OUTPUT);
		WireVL53L0X = &Wire;
		break;
	case 1:
		WireVL53L0X = &Wire1;
		break;
	case 2:
		WireVL53L0X = &Wire2;
		break;
	case 3:
		WireVL53L0X = &Wire3;
		break;
	case 4:
		WireVL53L0X = &Wire4;
		break;
	case 5:
		WireVL53L0X = &Wire5;
		break;
	default:
		return  mrb_bool_value(false);
		break;
	}

	WireVL53L0X->begin();

	sensor.init();
	sensor.setTimeout(ms);

	return mrb_bool_value(true);
}

//**************************************************
// より長い距離を測定するときに設定します: VL53L0X.range
//  VL53L0X.range(SignalRateLimit,PreRange,FinalRange)
//   SignalRateLimit: 信号レートの下限界(デフォルトは0.25MCPS(mega counts per second)
//　 PreRange: パルス周期のプレレンジ(12～18 default:14)
//   FinalRange: パルス周期のファイナルレンジ(8～14 default:10)
//
// 戻り値
//  true: 設定成功
//  false: 設定失敗
//**************************************************
mrb_value mrb_lv53l0x_setRange(mrb_state *mrb, mrb_value self)
{
	bool ret = true;
	int PreRange;
	int FinalRange;
	mrb_float SignalRateLimit;

	mrb_get_args(mrb, "fii", &SignalRateLimit, &PreRange, &FinalRange);

	// lower the return signal rate limit (default is 0.25 MCPS)
	ret = sensor.setSignalRateLimit(SignalRateLimit);

	// increase laser pulse periods (defaults are 14 and 10 PCLKs)
	ret &= sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodPreRange, PreRange);
	ret &= sensor.setVcselPulsePeriod(VL53L0X::VcselPeriodFinalRange, FinalRange);

	return mrb_bool_value(ret);
}

//**************************************************
// 測定に掛かる時間設定します: VL53L0X.timingBudget
//  VL53L0X.timingBudget(time)
//   time: usで指定します。最速値は20000usです。(デフォルトは33msです)
//        値を大きくすると測定精度が上がります。
//
// 戻り値
//  true: 設定成功
//  false: 設定失敗
//**************************************************
mrb_value mrb_lv53l0x_TimingBudget(mrb_state *mrb, mrb_value self)
{
	int budget;	//us

	mrb_get_args(mrb, "i", &budget);

	return mrb_bool_value(sensor.setMeasurementTimingBudget(budget));
}

//**************************************************
// 連続測定を開始します: VL53L0X.startContinuous
//  VL53L0X.startContinuous(time)
//   time: 測定間隔をmsで指定します。指定されない場合は、最短で測定を繰り返します。
//**************************************************
mrb_value mrb_lv53l0x_startContinuous(mrb_state *mrb, mrb_value self)
{
	// Start continuous back-to-back mode (take readings as
	// fast as possible).  To use continuous timed mode
	// instead, provide a desired inter-measurement period in
	// ms (e.g. sensor.startContinuous(100)).

	int ms;
	int n = mrb_get_args(mrb, "|i", &ms);

	if (n < 1) {
		sensor.startContinuous();
	}
	else {
		sensor.startContinuous(ms);
	}
	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// 連続測定している距離を読み出します: VL53L0X.readContinuous
//  VL53L0X.readContinuous()
//
// 戻り値
//  距離(mm)
//**************************************************
mrb_value mrb_lv53l0x_readContinuous(mrb_state *mrb, mrb_value self)
{
	return mrb_fixnum_value(sensor.readRangeContinuousMillimeters());
}

//**************************************************
// 距離を読み出します: VL53L0X.readSingle
//  VL53L0X.readSingle()
//
// 戻り値
//  距離(mm)
//**************************************************
mrb_value mrb_lv53l0x_readSingle(mrb_state *mrb, mrb_value self)
{
	return mrb_fixnum_value(sensor.readRangeSingleMillimeters());
}

//**************************************************
// 測定のタイムアウトをチェックします: VL53L0X.isTimeout
//  VL53L0X.isTimeout()
//
// 戻り値
//  tuue: タイムアウトした
//  false: タイムアウトしていない
//**************************************************
mrb_value mrb_lv53l0x_isTimeout(mrb_state *mrb, mrb_value self)
{
	return mrb_bool_value(sensor.timeoutOccurred());
}

//**************************************************
// ライブラリを定義します
//**************************************************
int vl53l0x_Init(mrb_state *mrb)
{
	struct RClass *vl53l0xModule = mrb_define_module(mrb, VL53L0X_CLASS);

	mrb_define_module_function(mrb, vl53l0xModule, "init", mrb_lv53l0x_init, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, vl53l0xModule, "range", mrb_lv53l0x_setRange, MRB_ARGS_REQ(3));
	mrb_define_module_function(mrb, vl53l0xModule, "timingBudget", mrb_lv53l0x_TimingBudget, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, vl53l0xModule, "startContinuous", mrb_lv53l0x_startContinuous, MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, vl53l0xModule, "readContinuous", mrb_lv53l0x_readContinuous, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, vl53l0xModule, "readSingle", mrb_lv53l0x_readSingle, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, vl53l0xModule, "isTimeout", mrb_lv53l0x_isTimeout, MRB_ARGS_NONE());

	return 1;
}
