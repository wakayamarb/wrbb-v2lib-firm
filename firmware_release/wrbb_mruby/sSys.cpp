/*
 * システム関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <EEPROM.h>
#include <reboot.h>

#include <mruby.h>
#include <mruby/string.h>
#include <mruby/array.h>
#include <mruby/variable.h>
#include <mruby/version.h>

#include <eeploader.h>

#include "../wrbb.h"

#include "sExec.h"
#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || BOARD == BOARD_P06
	#include "sSdCard.h"
	#include "sWiFi.h"
#endif

#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || (BOARD == BOARD_P06 && FIRMWARE == CITRUS)
	#include "sMp3.h"
#endif

#define EEPROMADDRESS	0xFF

extern volatile char ProgVer[];
extern char RubyFilename[];
extern char ExeFilename[];

//**************************************************
// 終了させます
//	System.exit()
//	エラー値がもどり、即終了
//**************************************************
mrb_value mrb_system_exit(mrb_state *mrb, mrb_value self)
{
	mrb_raise(mrb, mrb_class_get(mrb, "Sys#exit Called"), "Normal Completion");

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// 次に実行するスクリプトファイルをVmFilenameにセットします。
// System.setRun( filename )
//**************************************************
mrb_value mrb_system_setrun(mrb_state *mrb, mrb_value self)
{
mrb_value text;

	mrb_get_args(mrb, "S", &text);
	char *str = RSTRING_PTR(text);

	strcpy( (char*)RubyFilename, (char*)str );

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// システムのバージョンを取得します
// System.version([R])
// 引数があればmrubyのバーションを返す
//**************************************************
mrb_value mrb_system_version(mrb_state *mrb, mrb_value self)
{
int tmp;

	int n = mrb_get_args(mrb, "|i", &tmp);

	if( n>=1 ){
		//return mrb_const_get(mrb, mrb_obj_value(mrb->kernel_module), mrb_intern_lit(mrb, "MRUBY_VERSION"));
		return mrb_str_new_cstr(mrb, MRUBY_VERSION);
	}
	return mrb_str_new_cstr(mrb, (const char*)ProgVer);
}

//**************************************************
//フラッシュメモリに書き込みます
// System.push(address, buf, length)
//	address: 書き込み開始アドレス(0x0000～0x00ff)
//  buf: 書き込むデータ
//  length: 書き込むサイズ
// 戻り値
//  1:成功, 0:失敗
//**************************************************
mrb_value mrb_system_push(mrb_state *mrb, mrb_value self)
{
int		address;
mrb_value value;
int		len;
char	*str;

	mrb_get_args(mrb, "iSi", &address, &value, &len);

	str = RSTRING_PTR(value);

	if(address > EEPROMADDRESS){
		return mrb_fixnum_value( 0 );
	}

	int ret = 1;
	for(int i=0; i<len; i++){
		ret = EEPROM.write( (unsigned long)address, (unsigned char)str[i] );
		if(ret==-1){
			ret = 0;
			break;
		}
		address++;
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
//フラッシュメモリから読み出します
// System.pop(address, length)
//	address: 読み込みアドレス(0x0000～0x00ff)
//  length: 読み込みサイズ(MAX 32バイト)
// 戻り値
//  読み込んだデータ
//**************************************************
mrb_value mrb_system_pop(mrb_state *mrb, mrb_value self)
{
unsigned char str[32];
int		address;
int		len;

	mrb_get_args(mrb, "ii", &address, &len);

	if(len>32){
		len = 32;
	}

	if(address > EEPROMADDRESS){
		return mrb_str_new(mrb, (const char *)str, 0);
	}

	for(int i=0; i<len; i++){
		str[i] = EEPROM.read( (unsigned long)address );
		address++;
	}

	return mrb_str_new(mrb, (const char *)str, len);
}

//**************************************************
// ファイルローダーを呼び出します
// System.fileload()
//**************************************************
mrb_value mrb_system_fileload(mrb_state *mrb, mrb_value self)
{
	//ファイルローダーの呼び出し
	if(fileloader((const char*)ProgVer,MRUBY_VERSION) == 1){
		mrb_full_gc(mrb);	//強制GCを入れる
		//強制終了
		mrb_raise(mrb, mrb_class_get(mrb, "Sys#exit Called"), "Normal Completion");
	}

	mrb_full_gc(mrb);	//強制GCを入れる

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// リセットします
// System.reset()
//**************************************************
mrb_value mrb_system_reset(mrb_state *mrb, mrb_value self)
{
	system_reboot( REBOOT_USERAPP );	//リセット後にユーザアプリを起動する

	return mrb_nil_value();	//戻り値は無しですよ。
}

//**************************************************
// 実行しているmrbファイルパスを取得します: System.getMrbPath
// System.getMrbPath()
//戻り値
// 実行しているmrbファイルパス
//**************************************************
mrb_value mrb_system_getmrbpath(mrb_state *mrb, mrb_value self)
{
	return mrb_str_new_cstr(mrb, (const char*)ExeFilename);
}

//**************************************************
// SDカードを使えるようにします
//**************************************************
mrb_value Is_useSD(mrb_state *mrb, mrb_value self, int mode)
{
int ret = 0;

#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || BOARD == BOARD_P06
	ret = sdcard_Init(mrb);		//SDカード関連メソッドの設定
#endif

	return (mode == 0?mrb_fixnum_value(ret):mrb_bool_value(ret == 1));
}

//**************************************************
// SDカードを使えるようにします: System.useSD
// System.useSD()
//戻り値
// 0:使用不可, 1:使用可能
//**************************************************
mrb_value mrb_system_useSD(mrb_state *mrb, mrb_value self)
{
	return Is_useSD(mrb, self, 0);
}

//**************************************************
// WiFiモジュールESP8266ボードを使えるようにします
//**************************************************
mrb_value Is_useWiFi(mrb_state *mrb, mrb_value self, int mode)
{
int ret = 0;

#if FIRMWARE == SDWF || BOARD == BOARD_P05 || BOARD == BOARD_P06
	ret = esp8266_Init(mrb);		//ESP8266ボード関連メソッドの設定
#endif

	return (mode == 0?mrb_fixnum_value(ret):mrb_bool_value(ret == 1));
}

//**************************************************
// WiFiモジュールESP8266ボードを使えるようにします: System.useWiFi
// System.useWiFi()
//戻り値
// 0:使用不可, 1:使用可能
//**************************************************
mrb_value mrb_system_useWiFi(mrb_state *mrb, mrb_value self)
{
	return Is_useWiFi(mrb, self, 0);
}

//**************************************************
// MP3再生を行えるようにします
//**************************************************
mrb_value Is_useMp3(mrb_state *mrb, mrb_value self, int mode)
{
int ret = 0;

#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || (BOARD == BOARD_P06 && FIRMWARE == CITRUS)
	ret = mp3_Init(mrb);		//MP3関連メソッドの設定
#endif

	return (mode == 0?mrb_fixnum_value(ret):mrb_bool_value(ret == 1));
}

//**************************************************
// MP3再生を行えるようにします: System.useMP3
// System.useMP3(PausePin, StopPin)
//  PausePin: 再生中の一時停止に使用するピン番号です。LOWになると一時停止/再開を繰り返します
//  StopPin:  再生を止めるときに使用するピン番号です。LOWになると停止します
//
//戻り値
// 0:使用不可, 1:使用可能
//**************************************************
mrb_value mrb_system_useMp3(mrb_state *mrb, mrb_value self)
{
	return Is_useMp3(mrb, self, 0);
}

//**************************************************
// useで指定したクラスを使用できるようにします
//**************************************************
mrb_value Is_use(mrb_state *mrb, mrb_value self, int mode)
{
mrb_value vName, vOptions;
char	*strName;
int ret = 0;

	int n = mrb_get_args(mrb, "S|A", &vName, &vOptions);

	strName = RSTRING_PTR(vName);

	if(strcmp(strName, SD_CLASS) == 0){
		if(mode == 0){
			return Is_useSD(mrb, self, 0);
		}
		else{
			return Is_useSD(mrb, self, 1);
		}
	}
	else if(strcmp(strName, WIFI_CLASS) == 0){
		if(mode == 0){
			return Is_useWiFi(mrb, self, 0);
		}
		else{
			return Is_useWiFi(mrb, self, 1);
		}
	}
#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || (BOARD == BOARD_P06 && FIRMWARE == CITRUS)
	else if(strcmp(strName, MP3_CLASS) == 0){
		if(n == 1){
			return (mode == 0?mrb_fixnum_value(ret):mrb_bool_value(ret == 1));
		}

		n = RARRAY_LEN( vOptions );
		int pp, sp;
		if(n >= 2){
			pp = mrb_fixnum(mrb_ary_ref(mrb, vOptions, 0));
			sp = mrb_fixnum(mrb_ary_ref(mrb, vOptions, 1));
			ret = mp3_Init(mrb, pp, sp);		//MP3関連メソッドの設定
		}
	}
#endif
	return (mode == 0?mrb_fixnum_value(ret):mrb_bool_value(ret == 1));
}

//**************************************************
// 追加クラスを使用できるようにします: System.use?
// System.use?(ClassName[,Options])
//  ClassName: クラス名です。'SD'、'WiFi、'MP3'のいずれかです
//  Options: オプションの配列です。
//           SDはオプション無し。
//           WiFiはオプション無し。
//           MP3は再生中の一時停止に使用するピン番号と再生を止めるときに使用するピン番号の配列を指定します。例) [3,4]
//
//戻り値
// false:使用不可, true:使用可能
//**************************************************
mrb_value mrb_system_use_p(mrb_state *mrb, mrb_value self)
{
	return Is_use(mrb, self, 1);
}

//**************************************************
// 追加クラスを使用できるようにします: System.use
// System.use(ClassName[,Options])
//  ClassName: クラス名です。'SD'、'WiFi、'MP3'のいずれかです
//  Options: オプションの配列です。
//           SDはオプション無し。
//           WiFiはオプション無し。
//           MP3は再生中の一時停止に使用するピン番号と再生を止めるときに使用するピン番号の配列を指定します。例) [3,4]
//
//戻り値
// 0:使用不可, 1:使用可能
//**************************************************
mrb_value mrb_system_use(mrb_state *mrb, mrb_value self)
{
	return Is_use(mrb, self, 0);
}

//**************************************************
// ライブラリを定義します
//**************************************************
void sys_Init(mrb_state *mrb)
{
	struct RClass *systemModule = mrb_define_module(mrb, "System");

	mrb_define_module_function(mrb, systemModule, "exit", mrb_system_exit, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, systemModule, "reset", mrb_system_reset, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, systemModule, "setrun", mrb_system_setrun, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, systemModule, "version", mrb_system_version, MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, systemModule, "push", mrb_system_push, MRB_ARGS_REQ(3));
	mrb_define_module_function(mrb, systemModule, "pop", mrb_system_pop, MRB_ARGS_REQ(2));

	mrb_define_module_function(mrb, systemModule, "fileload", mrb_system_fileload, MRB_ARGS_NONE());

	mrb_define_module_function(mrb, systemModule, "useSD", mrb_system_useSD, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, systemModule, "useWiFi", mrb_system_useWiFi, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, systemModule, "useMP3", mrb_system_useMp3, MRB_ARGS_OPT(2));

	mrb_define_module_function(mrb, systemModule, "use", mrb_system_use,  MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, systemModule, "use?", mrb_system_use_p,  MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));

	//mrb_define_module_function(mrb, systemModule, "useSD?", mrb_system_useSD_p, MRB_ARGS_NONE());
	//mrb_define_module_function(mrb, systemModule, "useWiFi?", mrb_system_useWiFi_p, MRB_ARGS_NONE());
	//mrb_define_module_function(mrb, systemModule, "useMP3?", mrb_system_useMp3_p, MRB_ARGS_OPT(2));

	mrb_define_module_function(mrb, systemModule, "getMrbPath", mrb_system_getmrbpath, MRB_ARGS_NONE());
}
