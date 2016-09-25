/*
 * 呼び出し実行モジュールプログラム
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <string.h>

#include <mruby.h>
#include <mruby/irep.h>
#include <mruby/string.h>
#include <mruby/variable.h>
#include <mruby/error.h>
#include <mruby/array.h>

#include <eepfile.h>
#include <eeploader.h>

#include "../wrbb.h"
#include "sExec.h"
#include "sKernel.h"
#include "sSys.h"
#include "sSerial.h"
#include "sMem.h"
#include "sI2c.h"
#include "sServo.h"

#if REALTIMECLOCK
	#include "sRtc.h"
#endif

//#if FIRMWARE == JAM
//	#include "sPanCake.h"
//#endif
//

//バージョンのセット
volatile char	ProgVer[] = {FIRMWARE_TEXT BOARDNUM WRBB_VERSION BYTECODE_TEXT CPURAM};

extern char RubyStartFileName[];
extern char RubyFilename[];
extern char ExeFilename[];
extern bool SdClassFlag;

uint8_t RubyCode[RUBY_CODE_SIZE];	//静的にRubyコード領域を確保する

//**************************************************
//  スクリプト言語を実行します
//**************************************************
bool RubyRun( void )
{
bool notFinishFlag = true;

	//DEBUG_PRINT("mrb_open", "Before");
	mrb_state *mrb = mrb_open();
	DEBUG_PRINT("mrb_open", "After");
	
	if(mrb == NULL){
		Serial.println( "Can not Open mrb!!" );
		return false;
	}

	kernel_Init(mrb);	//カーネル関連メソッドの設定
	sys_Init(mrb);		//システム関連メソッドの設定
	serial_Init(mrb);	//シリアル通信関連メソッドの設定
	mem_Init(mrb);		//ファイル関連メソッドの設定
	i2c_Init(mrb);		//I2C関連メソッドの設定
	servo_Init(mrb);	//サーボ関連メソッドの設定

	//classtest_Init(mrb);

#if REALTIMECLOCK
	rtc_Init(mrb);		//RTC関連メソッドの設定
#endif

//#if FIRMWARE == JAM
//	pancake_Init(mrb);		//PanCake関連メソッドの設定
//#endif


	DEBUG_PRINT("RubyFilename",RubyFilename);

	strcpy( ExeFilename, RubyFilename );		//実行するファイルをExeFilename[]に入れる。
	//strcpy( RubyFilename, RubyStartFileName );	//とりあえず、RubyFilename[]をRubyStartFileName[]に初期化する。

	RubyFilename[0] = 0;						//Rubyファイル名をクリアする。System.setRun()やFileloaderでセットされ無い限り何も入っていない

	if(ExeFilename[0] == 0){
		mrb_close(mrb);

		DEBUG_PRINT("ExeFilename","NULL");
		return false;
	}

	FILEEEP fpj;
	FILEEEP *fp = &fpj;
	if(EEP.fopen(fp, ExeFilename, EEP_READ) == -1){
		char az[50];
		sprintf( az,  "%s is not Open!!", ExeFilename );
		Serial.println( az );
		mrb_close(mrb);
		return false;
	}

	//mrbファイルチェックを行う
	//int mrbFlag = 0;
	char he[8];
	for( int i=0; i<8; i++ ){	he[i] = EEP.fread(fp);	}

	if( !(he[0]=='R' && he[1]=='I'
	&& he[2]=='T' && he[3]=='E'
#if BYTECODE == BYTE_CODE2
	&& he[4]=='0' && he[5]=='0'
	&& he[6]=='0' && he[7]=='2'
#elif BYTECODE == BYTE_CODE3
	&& he[4]=='0' && he[5]=='0'
	&& he[6]=='0' && he[7]=='3'
#endif
	)){
		char az[50];
		sprintf( az,  "%s is not mrb file!!", ExeFilename );
		Serial.println( az );

		EEP.fclose(fp);
		mrb_close(mrb);
		return false;
	}

	//先頭にする
	EEP.fseek(fp, 0, EEP_SEEKTOP );

	//ファイルサイズを取得する
	unsigned long tsize = EEP.ffilesize(ExeFilename);

	if( tsize>RUBY_CODE_SIZE ){
		char az[50];
		sprintf( az,  "%s size is greater than %lu.", ExeFilename, RUBY_CODE_SIZE );
		Serial.println( az );
		mrb_close(mrb);
		return false;
	}

	RubyCode[0] = 0;
	unsigned long pos = 0;
	while( !EEP.fEof(fp) ){
		RubyCode[pos] = EEP.fread(fp);
		pos++;
	}
	EEP.fclose(fp);

	DEBUG_PRINT("mruby", "START");

	int arena = mrb_gc_arena_save(mrb);

	//mrubyを実行します
	mrb_load_irep( mrb, (const uint8_t *)RubyCode);

	if( mrb->exc ){
		//struct RString *str;
		char *s;
		int len;

		mrb_value obj = mrb_funcall(mrb, mrb_obj_value(mrb->exc), "inspect", 0);

		if (mrb_string_p(obj)) {
			s = RSTRING_PTR(obj);
			len = RSTRING_LEN(obj);

			const char *e = "Sys#exit";	//Sys#exitだったら正常終了ということ。
			int k = 8;		// ↑が8文字なので。
			int j = 0;
			for( int i=0; i<len; i++ ){
				if(*(s + i) == *(e + j)){
					j++;
					if(j == k){ break; }
				}
				else{
					j = 0;
				}
			}

			if( j<8 ){
				Serial_print_error(mrb, obj);
				notFinishFlag = false;
			}
		}
	}
	mrb->exc = 0;
	mrb_gc_arena_restore(mrb, arena);

	mrb_close(mrb);

	DEBUG_PRINT("mruby", "END");

	SdClassFlag = false;

	return notFinishFlag;
}

//**************************************************
// エラーメッセージ
//**************************************************
void Serial_print_error(mrb_state *mrb, mrb_value obj)
{
	Serial.println(RSTRING_PTR(obj));

	//mrb_value exc = mrb_obj_value(mrb->exc);
	mrb_value backtrace = mrb_get_backtrace(mrb);

	int j = 0;
	for (mrb_int n = mrb_ary_len(mrb, backtrace); j < n; ++j) {
		mrb_value v = mrb_ary_ref(mrb, backtrace, j);
		Serial.println(RSTRING_PTR(v));
	}
}

//**************************************************
// ピンモードを入力に初期化します
//**************************************************
void pinModeInit()
{
    pinMode(RB_LED, OUTPUT);

    pinMode(RB_PIN0, INPUT);
    pinMode(RB_PIN1, INPUT);
    pinMode(RB_PIN2, INPUT);
    pinMode(RB_PIN3, INPUT);
    pinMode(RB_PIN4, INPUT);
    pinMode(RB_PIN5, INPUT);
    pinMode(RB_PIN6, INPUT);
    pinMode(RB_PIN7, INPUT);
    pinMode(RB_PIN8, INPUT);
    pinMode(RB_PIN9, INPUT);
    pinMode(RB_PIN10, INPUT);
    pinMode(RB_PIN11, INPUT);
    pinMode(RB_PIN12, INPUT);
    pinMode(RB_PIN13, INPUT);
    pinMode(RB_PIN14, INPUT);
    pinMode(RB_PIN15, INPUT);
    pinMode(RB_PIN16, INPUT);
    pinMode(RB_PIN17, INPUT);
    pinMode(RB_PIN18, INPUT);
    pinMode(RB_PIN19, INPUT);

	pinMode(RB_PIN20, INPUT);
    pinMode(RB_PIN21, INPUT);
    pinMode(RB_PIN22, INPUT);
    pinMode(RB_PIN23, INPUT);
    pinMode(RB_PIN24, INPUT);
	pinMode(RB_PIN25, INPUT);
    pinMode(RB_PIN26, INPUT);
    pinMode(RB_PIN27, INPUT);
    pinMode(RB_PIN30, INPUT);
    pinMode(RB_PIN31, INPUT);
}