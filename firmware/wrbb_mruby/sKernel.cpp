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
// WRBB - SAKURAピン番コンバート
//**************************************************
int wrb2sakura(int pin)
{
int ret = 0;

	switch(pin){
	case 0:
		ret = RB_PIN0;
		break;
	case 1:
		ret = RB_PIN1;
		break;
	case 2:
		ret = RB_PIN2;
		break;
	case 3:
		ret = RB_PIN3;
		break;
	case 4:
		ret = RB_PIN4;
		break;
	case 5:
		ret = RB_PIN5;
		break;
	case 6:
		ret = RB_PIN6;
		break;
	case 7:
		ret = RB_PIN7;
		break;
	case 8:
		ret = RB_PIN8;
		break;
	case 9:
		ret = RB_PIN9;
		break;
	case 10:
		ret = RB_PIN10;
		break;
	case 11:
		ret = RB_PIN11;
		break;
	case 12:
		ret = RB_PIN12;
		break;
	case 13:
		ret = RB_PIN13;
		break;
	case 14:
		ret = RB_PIN14;
		break;
	case 15:
		ret = RB_PIN15;
		break;
	case 16:
		ret = RB_PIN16;
		break;
	case 17:
		ret = RB_PIN17;
		break;
	case 18:
		ret = RB_PIN18;
		break;
	case 19:
		ret = RB_PIN19;
		break;

	case 20:
		ret = RB_PIN20;
		break;
	case 21:
		ret = RB_PIN21;
		break;
	case 22:
		ret = RB_PIN22;
		break;
	case 23:
		ret = RB_PIN23;
		break;
	case 24:
		ret = RB_PIN24;
		break;
	case 25:
		ret = RB_PIN25;
		break;
	case 26:
		ret = RB_PIN26;
		break;
	case 27:
		ret = RB_PIN27;
		break;
	case 30:
		ret = RB_PIN30;
		break;
	case 31:
		ret = RB_PIN31;
		break;
	case 33:
		ret = RB_PIN33;
		break;

	default:
		ret = 55;
		break;
	}

return ret;
}

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
	digitalWrite( wrb2sakura(pin), value );

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

	pinMode( wrb2sakura(pin), value );

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

	value = digitalRead(wrb2sakura(pin));

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

	if(pin == 4 || pin >= 20){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	analogWrite(wrb2sakura(pin), value );

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

	if(pin == 4 || pin >= 20){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	noTone(wrb2sakura(pin));

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

	if(pin == 4 || pin >= 20){
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	dura = n < 3 ? 0 : dura;

	if( freq>=2 && freq<=62500 ){
		tone(wrb2sakura(pin), freq, dura);
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
