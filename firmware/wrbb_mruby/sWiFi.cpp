/*
 * ESP-WROOM-02関連
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
#include <mruby/string.h>

#include "../wrbb.h"
#include "sKernel.h"
#include "sSerial.h"

extern HardwareSerial *RbSerial[];		//0:Serial(USB), 1:Serial1, 2:Serial3, 3:Serial2, 4:Serial6 5:Serial7

#define  WIFI_SERIAL	3
#define  WIFI_BAUDRATE	115200
//#define  WIFI_CTS		15
#define  WIFI_WAIT_MSEC	10000

unsigned char WiFiData[256];
int WiFiRecvOutlNum = -1;	//ESP8266からの受信を出力するシリアル番号: -1の場合は出力しない。

//**************************************************
// OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、
// 指定されたシリアルポートに出力します
//
// 1:受信した, 0:受信できなかった
//**************************************************
int getData(int wait_msec)
{
//char f[16];
unsigned long times;
int c;
int okt = 0;
int ert = 0;
int len = 0;
int n = 0;

	//DEBUG_PRINT("getData", a);

	WiFiData[0] = 0;
	times = millis();
	while(n < 256){
		//digitalWrite(wrb2sakura(WIFI_CTS), 0);	//送信許可

		//wait_msec 待つ
		if(millis() - times > wait_msec){
			DEBUG_PRINT("WiFi get Data","Time OUT");
			WiFiData[n] = 0;
			return 0;
		}

		while(len = RbSerial[WIFI_SERIAL]->available())
		{
			for(int i=0; i<len; i++){
				c = RbSerial[WIFI_SERIAL]->read();

				//指定のシリアルポートに出す設定であれば、受信値を出力します
				if(WiFiRecvOutlNum >= 0){
					RbSerial[WiFiRecvOutlNum]->write((unsigned char)c);
				}

				WiFiData[n] = c;
				n++;
				//DEBUG_PRINT("c",c);

				if(c == 'O'){
					okt++;
					ert++;
				}
				else if(c == 'K'){
					okt++;
				}
				else if(c == 0x0d){
					ert++;
					okt++;
				}
				else if(c == 0x0a){
					ert++;
					okt++;
					if(okt == 4 || ert == 7){
						WiFiData[n] = 0;
						n = 256;
						break;
					}
					else{
						ert = 0;
						okt = 0;
					}
				}
				else if(c == 'E' || c == 'R'){
					ert++;
				}
				else{
					okt = 0;
					ert = 0;
				}
			}
			times = millis();
		}
	}
	//digitalWrite(wrb2sakura(WIFI_CTS), 0);	//送信許可
	return 1;
}

//**************************************************
// ステーションモードの設定: WiFi.cwmode
//  WiFi.cwmode(mode)
//  mode: 1:Station, 2:SoftAP, 3:Station + SoftAP
//**************************************************
mrb_value mrb_wifi_Cwmode(mrb_state *mrb, mrb_value self)
{
int	mode;

	mrb_get_args(mrb, "i", &mode);

	RbSerial[WIFI_SERIAL]->print("AT+CWMODE=");
	RbSerial[WIFI_SERIAL]->println(mode);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読か、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// コマンド応答のシリアル出力設定: WiFi.sout
//  WiFi.sout( mode[,serialNumber] )
//	mode: 0:出力しない, 1:出力する
//  serialNumber: 出力先のシリアル番号
//**************************************************
mrb_value mrb_wifi_Sout(mrb_state *mrb, mrb_value self)
{
int mode;
int num = -1;

	int n = mrb_get_args(mrb, "i|i", &mode, &num);

	if(mode == 0){
		WiFiRecvOutlNum = -1;
	}
	else{
		if( n>=2 ){
			if(num >= 0){
				WiFiRecvOutlNum = num;
			}
		}
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// ATコマンドの送信: WiFi.at
//  WiFi.at( command[, mode] )
//	commnad: ATコマンド内容
//  mode: 0:'AT+'を自動追加する、1:'AT+'を自動追加しない
//**************************************************
mrb_value mrb_wifi_at(mrb_state *mrb, mrb_value self)
{
mrb_value text;
int mode = 0;

	int n = mrb_get_args(mrb, "S|i", &text, &mode);

	char *s = RSTRING_PTR(text);
	int len = RSTRING_LEN(text);

	if(n <= 1 || mode == 0){
		RbSerial[WIFI_SERIAL]->print("AT+");
	}

	for(int i=0; i<254; i++){
		if( i >= len){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[len] = 0;

	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);
	//DEBUG_PRINT("WiFi.at",(const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読か、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// WiFi接続します: WiFi.cwjap
//  WiFi.cwjap(SSID,Passwd)
//  SSID: WiFiのSSID
//  Passwd: パスワード
//**************************************************
mrb_value mrb_wifi_Cwjap(mrb_state *mrb, mrb_value self)
{
mrb_value ssid;
mrb_value passwd;

	mrb_get_args(mrb, "SS", &ssid, &passwd);

	char *s = RSTRING_PTR(ssid);
	int slen = RSTRING_LEN(ssid);

	char *p = RSTRING_PTR(passwd);
	int plen = RSTRING_LEN(passwd);

	RbSerial[WIFI_SERIAL]->print("AT+CWJAP=");
	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for(int i=0; i<254; i++){
		if( i >= slen){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[slen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0x2C;		//-> ,
	WiFiData[2] = 0x22;		//-> "
	WiFiData[3] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for(int i=0; i<254; i++){
		if( i >= plen){ break; }
		WiFiData[i] = p[i];
	}
	WiFiData[plen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読か、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// IPアドレスとMACアドレスの表示: WiFi.cifsr
//  WiFi.cwjap()
//**************************************************
mrb_value mrb_wifi_Cifsr(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+CIFSR");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読か、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// USBポートとESP8266をシリアルで直結します: WiFi.bypass
//  WiFi.bypass()
// リセットするまで、処理は戻りません。
//**************************************************
mrb_value mrb_wifi_bypass(mrb_state *mrb, mrb_value self)
{
	int len0, len1,len;
	int retCnt = 0;

	while(true){
		len0 = RbSerial[0]->available();
		len1 = RbSerial[WIFI_SERIAL]->available();

		if(len0 > 0){
			len = len0<256 ? len0 : 256;

			for(int i=0; i<len; i++){
				WiFiData[i] = (unsigned char)RbSerial[0]->read();
			}
			RbSerial[WIFI_SERIAL]->write( WiFiData, len );
		}

		if(len1 > 0){
			len = len1<256 ? len1 : 256;
			
			for(int i=0; i<len; i++){
				WiFiData[i] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
			}
	        RbSerial[0]->write( WiFiData, len );
		}
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// ライブラリを定義します
//**************************************************
int esp8266_Init(mrb_state *mrb)
{	
	//ESP8266からの受信を出力しないに設定
	WiFiRecvOutlNum = -1;

	//CTS用にPIN15をOUTPUTに設定します
	//pinMode(wrb2sakura(WIFI_CTS), 1);
	//digitalWrite(wrb2sakura(WIFI_CTS), 1);

	//WiFiのシリアル3を設定
	//シリアル通信の初期化をします
	RbSerial[WIFI_SERIAL]->begin(WIFI_BAUDRATE);
	int len;

	//受信バッファを空にします
	while((len = RbSerial[WIFI_SERIAL]->available()) > 0){
		RbSerial[0]->print(len);
		for(int i=0; i<len; i++){
			RbSerial[WIFI_SERIAL]->read();
		}
	}

	//ECHOオフコマンドを送信する
	RbSerial[WIFI_SERIAL]->println("ATE0");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読む
	if(getData(500) == 0){
		return 0;
	}
		
	struct RClass *wifiModule = mrb_define_module(mrb, "WiFi");

	mrb_define_module_function(mrb, wifiModule, "at", mrb_wifi_at, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "sout", mrb_wifi_Sout, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "cwmode", mrb_wifi_Cwmode, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, wifiModule, "cwjap", mrb_wifi_Cwjap, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, wifiModule, "connect", mrb_wifi_Cwjap, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, wifiModule, "cifsr", mrb_wifi_Cifsr, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, wifiModule, "config", mrb_wifi_Cifsr, MRB_ARGS_NONE());

	mrb_define_module_function(mrb, wifiModule, "bypass", mrb_wifi_bypass, MRB_ARGS_NONE());

	return 1;

	//mrb_define_module_function(mrb, pancakeModule, "clear", mrb_pancake_Clear, MRB_ARGS_REQ(1));
	//mrb_define_module_function(mrb, pancakeModule, "line", mrb_pancake_Line, MRB_ARGS_REQ(5));
	//mrb_define_module_function(mrb, pancakeModule, "circle", mrb_pancake_Circle, MRB_ARGS_REQ(4));
	//mrb_define_module_function(mrb, pancakeModule, "stamp", mrb_pancake_Stamp, MRB_ARGS_REQ(4));
	//mrb_define_module_function(mrb, pancakeModule, "stamp1", mrb_pancake_Stamp1, MRB_ARGS_REQ(4));
	//mrb_define_module_function(mrb, pancakeModule, "image", mrb_pancake_Image, MRB_ARGS_REQ(1));
	//mrb_define_module_function(mrb, pancakeModule, "video", mrb_pancake_Video, MRB_ARGS_REQ(1));
	//mrb_define_module_function(mrb, pancakeModule, "sound", mrb_pancake_Sound, MRB_ARGS_REQ(8));
	//mrb_define_module_function(mrb, pancakeModule, "sound1", mrb_pancake_Sound1, MRB_ARGS_REQ(3));
	//mrb_define_module_function(mrb, pancakeModule, "reset", mrb_pancake_Reset, MRB_ARGS_NONE());
	//mrb_define_module_function(mrb, pancakeModule, "out", mrb_pancake_Out, MRB_ARGS_REQ(1));

	//struct RClass *spriteModule = mrb_define_module(mrb, "Sprite");
	//mrb_define_module_function(mrb, spriteModule, "start", mrb_pancake_Start, MRB_ARGS_REQ(1));
	//mrb_define_module_function(mrb, spriteModule, "create", mrb_pancake_Create, MRB_ARGS_REQ(2));
	//mrb_define_module_function(mrb, spriteModule, "move", mrb_pancake_Move, MRB_ARGS_REQ(3));
	//mrb_define_module_function(mrb, spriteModule, "flip", mrb_pancake_Flip, MRB_ARGS_REQ(2));
	//mrb_define_module_function(mrb, spriteModule, "rotate", mrb_pancake_Rotate, MRB_ARGS_REQ(2));
	//mrb_define_module_function(mrb, spriteModule, "user", mrb_pancake_User, MRB_ARGS_REQ(3));

	//struct RClass *musicModule = mrb_define_module(mrb, "Music");
	//mrb_define_module_function(mrb, musicModule, "score", mrb_pancake_Score, MRB_ARGS_REQ(4));
	//mrb_define_module_function(mrb, musicModule, "play", mrb_pancake_Play, MRB_ARGS_REQ(1));*/
}
