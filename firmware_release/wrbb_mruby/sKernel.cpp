/*
 * カーネル関連
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

#include "../wrbb.h"


//**************************************************
// デジタルライト
//	digitalWrite(pin, value)
//	pin
//		ピンの番号
//	value
//		0: LOW
//		1: HIGH
//**************************************************
mrb_value mrb_kernel_digitalWrite(mrb_state *mrb, mrb_value self)
{
int pin, value;

	mrb_get_args(mrb, "ii", &pin, &value);


	if(pin >=20 && pin <= 30){
		return mrb_nil_value();			//戻り値は無しですよ。
	}
	digitalWrite( pin, value );

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// PINのモード設定
//	pinMode(pin, mode)
//  pin
//		ピンの番号
//	mode
//		0: INPUTモード
//		1: OUTPUTモード
//**************************************************
mrb_value mrb_kernel_pinMode(mrb_state *mrb, mrb_value self)
{
int pin, value;

	mrb_get_args(mrb, "ii", &pin, &value);

	pinMode( pin, value );

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// ディレイ 強制GCを行っています
//	delay(value)
//	value
//		時間(ms)
//**************************************************
mrb_value mrb_kernel_delay(mrb_state *mrb, mrb_value self)
{
int value;

	mrb_get_args(mrb, "i", &value);

	//試しに強制gcを入れて見る
	mrb_full_gc(mrb);

	if(value >0 ){
		delay( value );
	}

	return mrb_nil_value();			//戻り値は無しですよ。
}


//**************************************************
// ミリ秒を取得します: millis
//	millis()
// 戻り値
//	起動してからのミリ秒数
//**************************************************
mrb_value mrb_kernel_millis(mrb_state *mrb, mrb_value self)
{	
	return mrb_fixnum_value( (mrb_int)millis() );
}

//**************************************************
// マイクロ秒を取得します: micros
//	micros()
// 戻り値
//	起動してからのマイクロ秒数
//**************************************************
mrb_value mrb_kernel_micros(mrb_state *mrb, mrb_value self)
{
	return mrb_fixnum_value( (mrb_int)micros() );
}

//**************************************************
// デジタルリード: digitalRead
//	digitalRead(pin)
//	pin: ピンの番号
//	
//		0:LOW
//		1:HIGH
//**************************************************
mrb_value mrb_kernel_digitalRead(mrb_state *mrb, mrb_value self)
{
int pin, value;

	mrb_get_args(mrb, "i", &pin);

	value = digitalRead(pin);

	return mrb_fixnum_value( value );
}


//**************************************************
// アナログリファレンス: analogReference
//	analogReference(mode)
//  アナログ入力で使われる基準電圧を設定します
//	mode: 0:DEFAULT：5.0V Arduino互換, 1:INTERNAL：1.1V 内蔵電圧, 2:EXTERNAL：AVREFピン供給電圧, 3:RAW12BIT：3.3V 12ビットA/D変換を行う
//**************************************************
mrb_value mrb_kernel_analogReference(mrb_state *mrb, mrb_value self)
{
int mode;

	mrb_get_args(mrb, "i", &mode);

	switch(mode){
	case DEFAULT:
		analogReference(DEFAULT);
		break;
	case INTERNAL:
		analogReference(INTERNAL);
		break;
	case EXTERNAL:
		analogReference(EXTERNAL);
		break;
	case RAW12BIT:
		analogReference(RAW12BIT);
		break;
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// アナログリード: analogRead
//	analogRead(pin)
//	pin: アナログの番号
//	
//		10ビットの値(0～1023)
//**************************************************
mrb_value mrb_kernel_analogRead(mrb_state *mrb, mrb_value self)
{
int anapin, value;

	mrb_get_args(mrb, "i", &anapin);

	value = analogRead( anapin );

	return mrb_fixnum_value( value );
}


////**************************************************
//// 出力ピンが並列接続されているピンとショートするかどうか調べます
////
//// true: 衝突している
//// false: 衝突していない。片方がINPUTである。
////**************************************************
//bool IsWritePinCollision(int pinSub)
//{
//	PinMode modeSub = getPinMode(wrb2sakura(pinSub));
//	if(modeSub == PinModeInput || modeSub== PinModeInputPullUp){
//		return false;
//	}
//	return true;
//}


//**************************************************
// PWM出力: pwm
//	pwm(pin, value)
//	pin: ピンの番号(0,1,7,8,11,23ピンがPWM可能)
//       ただし、23ピンは5ピンと24ピン短絡しているので、使用時は5ピンと24ピンをINPUTにしておく
//  value:	出力PWM比率(0～255)
//**************************************************
mrb_value mrb_kernel_pwm(mrb_state *mrb, mrb_value self)
{
int pin, value;

	mrb_get_args(mrb, "ii", &pin, &value);

	if(pin >= 20){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	analogWrite(pin, value );

	return mrb_nil_value();			//戻り値は無しですよ。
}

////**************************************************
//// PWM周波数設定: pwmHz
////	pwmHz(value)
////  value:	周波数(12～184999)Hz
////**************************************************
//mrb_value mrb_kernel_pwmHz(mrb_state *mrb, mrb_value self)
//{
//int value;
//
//	mrb_get_args(mrb, "i", &value);
//
//	if( value>=12 && value<18500 ){
//		analogWriteFrequency(value);
//	}
//
//	return mrb_nil_value();			//戻り値は無しですよ。
//}

//**************************************************
// トーン出力停止: noTone
//	noTone(pin)
//  pin: ピン番号
//**************************************************
mrb_value mrb_kernel_noTone(mrb_state *mrb, mrb_value self)
{
int pin;

	mrb_get_args(mrb, "i", &pin);

	if(pin >= 20){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	noTone(pin);

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// トーン出力: tone
//	tone(pin, frequency[,duration])
//  pin: ピン番号
//  frequency: 周波数(2～62500)Hz
//  duration: 出力を維持する時間[ms]。省略時、0指定時は出力し続ける。
//**************************************************
mrb_value mrb_kernel_tone(mrb_state *mrb, mrb_value self)
{
int pin;
int freq;
unsigned long dura;

	int n = mrb_get_args(mrb, "ii|i", &pin, &freq, &dura);

	if(pin >= 20){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	dura = n < 3 ? 0 : dura;

	if( freq>=2 && freq<=62500 ){
		tone(pin, freq, dura);
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// アナログDACピン初期化: initDac
//	initDac()
//**************************************************
mrb_value mrb_kernel_initDac(mrb_state *mrb, mrb_value self)
{
	setPinModeDac(RB_PIN9);
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// アナログDAC出力: analogDac
//	analogDac(value)
//  value:	10bit精度(0～4095)
//**************************************************
mrb_value mrb_kernel_analogDac(mrb_state *mrb, mrb_value self)
{
int value;

	mrb_get_args(mrb, "i", &value);

	if( value>=0 && value<4096 ){
		analogWriteDAC( RB_PIN9, value );
	}

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// LEDオンオフ: led
//	led(sw)
//**************************************************
mrb_value mrb_kernel_led(mrb_state *mrb, mrb_value self)
{
int value;

	mrb_get_args(mrb, "i", &value);

#if BOARD == BOARD_GR
	digitalWrite( PIN_LED0, value & 1 );
	digitalWrite( PIN_LED1, (value>>1) & 1 );
	digitalWrite( PIN_LED2, (value>>2) & 1 );
	digitalWrite( PIN_LED3, (value>>3) & 1 );
#else
	digitalWrite( RB_LED, value & 1 );
#endif

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// 乱数を得るための種を与えます: randomSeed
//  randomSeed(value)
//  value: 種となる値
//**************************************************
mrb_value mrb_kernel_randomSeed(mrb_state *mrb, mrb_value self)
{
unsigned int value;

	mrb_get_args(mrb, "i", &value);

	randomSeed(value);

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// 乱数を取得します: random
//  random([min,] max)
//  min: 乱数の取りうる最小値。省略可
//  max: 乱数の取りうる最大値
//**************************************************
mrb_value mrb_kernel_random(mrb_state *mrb, mrb_value self)
{
long value1,value2;

	int n = mrb_get_args(mrb, "i|i", &value1, &value2);

	if(n == 1){
		return mrb_fixnum_value( random(value1) );
	}

	return mrb_fixnum_value( random(value1, value2) );
}

//**************************************************
// 隠しコマンドです:  El_Psy.Congroo
//	El_Psy.Congroo()
//**************************************************
mrb_value mrb_El_Psy_congroo(mrb_state *mrb, mrb_value self)
{
	mrb_raise(mrb, mrb_class_get(mrb, "Sys#exit Called"), "Normal Completion");

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// ライブラリを定義します
//**************************************************
void kernel_Init(mrb_state *mrb)
{
	mrb_define_method(mrb, mrb->kernel_module, "pinMode", mrb_kernel_pinMode, MRB_ARGS_REQ(2));

	mrb_define_method(mrb, mrb->kernel_module, "digitalWrite", mrb_kernel_digitalWrite, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, mrb->kernel_module, "pwm", mrb_kernel_pwm, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, mrb->kernel_module, "digitalRead", mrb_kernel_digitalRead, MRB_ARGS_REQ(1));

	mrb_define_method(mrb, mrb->kernel_module, "analogReference", mrb_kernel_analogReference, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->kernel_module, "analogRead", mrb_kernel_analogRead, MRB_ARGS_REQ(1));

	mrb_define_method(mrb, mrb->kernel_module, "tone", mrb_kernel_tone, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));
	mrb_define_method(mrb, mrb->kernel_module, "noTone", mrb_kernel_noTone, MRB_ARGS_REQ(1));

	//mrb_define_method(mrb, mrb->kernel_module, "pwmHz", mrb_kernel_pwmHz, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->kernel_module, "initDac", mrb_kernel_initDac, MRB_ARGS_NONE());
	mrb_define_method(mrb, mrb->kernel_module, "analogDac", mrb_kernel_analogDac, MRB_ARGS_REQ(1));

	mrb_define_method(mrb, mrb->kernel_module, "delay", mrb_kernel_delay, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->kernel_module, "millis", mrb_kernel_millis, MRB_ARGS_NONE());
	mrb_define_method(mrb, mrb->kernel_module, "micros", mrb_kernel_micros, MRB_ARGS_NONE());

	mrb_define_method(mrb, mrb->kernel_module, "led", mrb_kernel_led, MRB_ARGS_REQ(1));

	mrb_define_method(mrb, mrb->kernel_module, "randomSeed", mrb_kernel_randomSeed, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, mrb->kernel_module, "random", mrb_kernel_random, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));

	struct RClass *El_PsyModule = mrb_define_module(mrb, "El_Psy");
	mrb_define_module_function(mrb, El_PsyModule, "Congroo", mrb_El_Psy_congroo, MRB_ARGS_NONE());



}
