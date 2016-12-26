/*
 * MP3関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <MsTimer2.h>
#include <WavMp3p.h>

#include <mruby.h>
#include <mruby/string.h>

#include "../wrbb.h"

#include "sMp3.h"

#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || BOARD == BOARD_P06
	#include "sSdCard.h"
#endif

#define SW_ON		0
#define SW_OFF		1

int PausePin = 3;		//再生中の一時停止に使用するピン番号です。LOWになると一時停止/再開を繰り返します
int StopPin = 4;		//再生を止めるときに使用するピン番号です。LOWになると停止します
bool SwLED = 0;			//再生中にLEDを点滅させるフラグ

WavMp3p *Wavmp3p = 0;

static void cyclic_handler()
{
	static int ct_pause = 0;

	if(SW_ON == digitalRead(PausePin)){
		if(ct_pause == 2){
			if(Wavmp3p->read_pause()){
				Wavmp3p->pause(0);
			}
			else{
				Wavmp3p->pause(1);
			}
			ct_pause++;
		}
		else if(ct_pause < 2){
			ct_pause++;
		}
	}
	else{
		ct_pause = 0;
	}

	static int ct_skip = 0;

	if(SW_ON == digitalRead(StopPin)){
		if(ct_skip == 2){
			Wavmp3p->skip();
			ct_skip++;
		}
		else if(ct_skip < 2){
			ct_skip++;
		}
	}
	else{
		ct_skip = 0;
	}

	static int ct_led = 0;

	if(SwLED){
		if(Wavmp3p->read_pause()){
			digitalWrite(RB_LED, HIGH);
			ct_led = 11;
		}
		else{
			if(ct_led > 10){
				if(digitalRead(RB_LED) == 0){
					digitalWrite(RB_LED, HIGH);
				}
				else{
					digitalWrite(RB_LED, LOW);
				}
				ct_led = 0;
			}
			ct_led++;
		}
	}
}

//**************************************************
//
// MP3関連に使えるピンであるかどうかを確認します
//
//**************************************************
bool chkCanUsePin(int pin)
{
bool ret = true;

	switch(pin){
	case RB_PIN0:
	case RB_PIN19:
	case RB_PIN2:
	case RB_PIN5:
	case RB_PIN7:
	case RB_PIN8:
	case RB_PIN11:
	case RB_PIN12:
	case RB_PIN13:
		ret = false;
		break;
	}
	//Serial.print(pin);
	//Serial.print(":");
	//Serial.println(ret);

	return ret;
}

//**************************************************
// MP3ファイルを再生します: MP3.play
//  MP3.play(filename)
//  filename: 再生するファイル名
//
//  エラー出たときは、その内容が返る
//**************************************************
mrb_value mrb_mp3_play(mrb_state *mrb, mrb_value self)
{
mrb_value vfpath;
char *fpath;

	mrb_get_args(mrb, "S", &vfpath);

	//ピンの初期化
   	Wavmp3p->init(44100);

	fpath = RSTRING_PTR(vfpath);

	//ポーズとストップ用の割り込み開始
	MsTimer2::start();

	char *res = Wavmp3p->play(fpath);

	//ポーズとストップ用の割り込み停止
	MsTimer2::stop();

	if(SwLED == true){
		digitalWrite(RB_LED, LOW);
	}

	return mrb_str_new_cstr(mrb, (const char*)res);
}

//**************************************************
// MP3再生中にLEDを点滅させるフラグ: MP3.led
//  MP3.led(sw)
//  sw: 0:何もしない、1:点滅させる
//
// 戻り値
// 無しです
//**************************************************
mrb_value mrb_mp3_led(mrb_state *mrb, mrb_value self)
{
int led = 0;

	mrb_get_args(mrb, "i", &led);

	if(led == 0){
		SwLED = false;
	}
	else{
		SwLED = true;
	}
	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// ライブラリを定義します
//**************************************************
int mp3_Init(mrb_state *mrb,int pausePin, int stopPin)
{
	PausePin = pausePin;
	StopPin = stopPin;

	//使用できるピンかどうかチェック
	if(PausePin == StopPin){
		return 0;
	}
	if(!chkCanUsePin(PausePin) || !chkCanUsePin(StopPin)){
		return 0;
	}

	//SDカードが利用可能か確かめます
	if(!sdcard_Init(mrb)){
		return 0;
	}

	//インスタンスを生成します。
	if(Wavmp3p == 0){
		Wavmp3p = new WavMp3p(44100);
	}

	pinMode(PausePin, INPUT_PULLUP);
	pinMode(StopPin, INPUT_PULLUP);

	MsTimer2::set(100, cyclic_handler);

	struct RClass *mp3Module = mrb_define_module(mrb, MP3_CLASS);

	mrb_define_module_function(mrb, mp3Module, "play", mrb_mp3_play, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, mp3Module, "led", mrb_mp3_led, MRB_ARGS_REQ(1));

	return 1;
}
//**************************************************
// ライブラリを定義します
//**************************************************
int mp3_Init(mrb_state *mrb)
{
int pausePin, stopPin;

	//ポーズ割り込みピン番号とスキップ割り込みピン番号を取得します
	mrb_get_args(mrb, "ii", &pausePin, &stopPin);

	return mp3_Init(mrb, pausePin, stopPin);
}
