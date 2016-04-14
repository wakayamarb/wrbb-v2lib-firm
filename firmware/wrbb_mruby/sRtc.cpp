/*
 * RTC関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <RTC.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/hash.h>

#include "../wrbb.h"
#include "sKernel.h"

RTC_TIMETYPE TimeRTC;

//**************************************************
// RTCの時計を取得します: Rtc.getTime
//  Rtc.getDateTime()
//
//  戻り値は以下の値が配列で返ります
//  year: 年(2000-2099)
//  mon: 月(1-12)
//  day: 日(1-31)
//  hour: 時(0-23)
//  min: 分(0-59)
//  second: 秒(0-59)
//  weekday: 曜日(0-6)0:日,1:月,2:火,3:水,4:木,5:金,6:土
//**************************************************
mrb_value mrb_rtc_getTime(mrb_state *mrb, mrb_value self)
{
mrb_value arv[7];

	if(rtc_get_time(&TimeRTC)){
		arv[0] = mrb_fixnum_value(TimeRTC.year + 2000);
		arv[1] = mrb_fixnum_value(TimeRTC.mon);
		arv[2] = mrb_fixnum_value(TimeRTC.day);
		arv[3] = mrb_fixnum_value(TimeRTC.hour);
		arv[4] = mrb_fixnum_value(TimeRTC.min);
		arv[5] = mrb_fixnum_value(TimeRTC.second);
		arv[6] = mrb_fixnum_value(TimeRTC.weekday);
	}
	else{
		arv[0] = mrb_fixnum_value(-1);
		arv[1] = mrb_fixnum_value(-1);
		arv[2] = mrb_fixnum_value(-1);
		arv[3] = mrb_fixnum_value(-1);
		arv[4] = mrb_fixnum_value(-1);
		arv[5] = mrb_fixnum_value(-1);
		arv[6] = mrb_fixnum_value(-1);
	}
	return mrb_ary_new_from_values(mrb, 7, arv);

//mrb_value hash = mrb_hash_new(mrb);
//
//	if(rtc_get_time(&TimeRTC)){
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "year"), mrb_fixnum_value(TimeRTC.year));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "mon"), mrb_fixnum_value(TimeRTC.mon));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "day"), mrb_fixnum_value(TimeRTC.day));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "hour"), mrb_fixnum_value(TimeRTC.hour));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "min"), mrb_fixnum_value(TimeRTC.min));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "second"), mrb_fixnum_value(TimeRTC.second));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "weekday"), mrb_fixnum_value(TimeRTC.weekday));
//	}
//	else{
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "year"), mrb_fixnum_value(-1));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "mon"), mrb_fixnum_value(-1));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "day"), mrb_fixnum_value(-1));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "hour"), mrb_fixnum_value(-1));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "min"), mrb_fixnum_value(-1));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "second"), mrb_fixnum_value(-1));
//		mrb_hash_set(mrb, hash, mrb_str_new_cstr(mrb, "weekday"), mrb_fixnum_value(-1));
//	}
//	return hash;
}

//**************************************************
// RTCの時計をセットします: Rtc.setTime
//  Rtc.setTime( array )
//  array: 年(0000-9999),月(1-12),日(1-31),時(0-23),分(0-59),秒(0-59)の配列
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//**************************************************
mrb_value mrb_rtc_setTime(mrb_state *mrb, mrb_value self)
{
mrb_value value;
int year, mon;

	mrb_get_args(mrb, "A", &value);

	if ( !mrb_array_p( value ) ){
		return mrb_fixnum_value(0);
	}

    int len = RARRAY_LEN( value );
	if(len < 6){
		return mrb_fixnum_value(0);
	}

	TimeRTC.year = mrb_fixnum(mrb_ary_ref(mrb, value, 0));
	TimeRTC.mon = mrb_fixnum(mrb_ary_ref(mrb, value, 1));
	TimeRTC.day = mrb_fixnum(mrb_ary_ref(mrb, value, 2));
	TimeRTC.hour = mrb_fixnum(mrb_ary_ref(mrb, value, 3));
	TimeRTC.min = mrb_fixnum(mrb_ary_ref(mrb, value, 4));
	TimeRTC.second = mrb_fixnum(mrb_ary_ref(mrb, value, 5));
	if(TimeRTC.mon <= 2){
		mon = TimeRTC.mon + 12;
		year = TimeRTC.year - 1;
	}
	else{
		mon = TimeRTC.mon;
		year = TimeRTC.year;
	}

	TimeRTC.weekday = (year + year/4 - year/100 + year/400 + (13 * mon + 8)/5 + TimeRTC.day) % 7;
	TimeRTC.year = TimeRTC.year % 100;
	
    return mrb_fixnum_value(rtc_set_time(&TimeRTC));
}

//**************************************************
// RTCを停止します: Rtc.deinit
//  Rtc.deinit()
//
// 戻り値は以下のとおり
//	0: 失敗
//	1: 成功
//**************************************************
mrb_value mrb_rtc_deinit(mrb_state *mrb, mrb_value self)
{
	return mrb_fixnum_value( rtc_deinit() );
}

//**************************************************
// RTCを起動します: Rtc.init
//  Rtc.init()
//
// 戻り値は以下のとおり
//	0: 起動失敗
//	1: 起動成功
//	2: RTCは既に起動していた(成功)
//**************************************************
mrb_value mrb_rtc_init(mrb_state *mrb, mrb_value self)
{
	return mrb_fixnum_value( rtc_init() );
}

//**************************************************
// ライブラリを定義します
//**************************************************
void rtc_Init(mrb_state *mrb)
{
	struct RClass *rtcModule = mrb_define_module(mrb, "Rtc");

	mrb_define_module_function(mrb, rtcModule, "init", mrb_rtc_init, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, rtcModule, "begin", mrb_rtc_init, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, rtcModule, "deinit", mrb_rtc_deinit, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, rtcModule, "setTime", mrb_rtc_setTime, MRB_ARGS_REQ(6));
	mrb_define_module_function(mrb, rtcModule, "getTime", mrb_rtc_getTime, MRB_ARGS_NONE());

}

//rtc_attach_alarm_handler
//概要 	アラーム発生時に処理する関数を登録します。
//文法 	rtc_attach_alarm_handler(void(*)(void) function)
//パラメータ 	function: 処理する関数
//戻り値 	なし
//
//rtc_set_alarm_time
//概要 	アラーム時間を設定します。
//文法 	int rtc_set_alarm_time (int hour, int min, int week_flag)
//パラメータ 	hour: 時
//min: 分
//week_flag: 曜日（複数指定の場合は論理和で指定）
//戻り値 	0:失敗、1:成功
//
//rtc_alarm_on
//概要 	アラームをONにします。
//文法 	rtc_alarm_on()
//パラメータ 	なし
//戻り値 	なし
//
//rtc_alarm_off
//概要 	アラームをOFFにします。
//文法 	rtc_alarm_off()
//パラメータ 	なし
//戻り値 	なし
//
