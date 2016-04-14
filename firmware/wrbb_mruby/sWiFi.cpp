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

extern int Ack_FE_mode;
extern RB_Serial rbserial[];

#define  WIFI_SERIAL	3
#define  WIFI_BAUDRATE	115200
#define  WIFI_CTS		15
#define  WIFI_WAIT_MSEC	10000

unsigned char WiFiData[256];
int WiFiRecvOutlNum = -1;	//ESP8266からの受信を出力するシリアル番号: -1の場合は出力しない。

//**************************************************
// OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、
// 指定されたシリアルポートに出力します
//**************************************************
void getData(int wait_msec)
{
char f[16];
unsigned long times;
int c;
int okt = 0;
int ert = 0;
int len = 0;

	//DEBUG_PRINT("getData", a);

	//受信バッファを空にします
	digitalWrite(wrb2sakura(WIFI_CTS), 0);	//送信許可
	while((len=sci_rxcount_ex(rbserial[WIFI_SERIAL].sci)) > 0){
		for(int i=0; i<len; i++){ Serial.read(); }
		delay(0);
	}

	WiFiData[0] = 0;
	for(int i=0; i<255; i++){

		digitalWrite(wrb2sakura(WIFI_CTS), 0);	//送信許可

		times = millis();
		while(!(len=sci_rxcount_ex(rbserial[WIFI_SERIAL].sci)))
		{
			//wait_msec 待つ
			if(millis() - times > wait_msec){
				DEBUG_PRINT("WiFi get Data","Time OUT");
				WiFiData[i + 1] = 0;
				return;
			}

			//DEBUG_PRINT("getData","DATA Waiting");
		}
		digitalWrite(wrb2sakura(WIFI_CTS), 1);	//送信許可しない

		c = rbserial[WIFI_SERIAL].serial->read();
		
		//指定のシリアルポートに出す設定であれば、受信値を出力します
		if(WiFiRecvOutlNum >= 0){
			rbserial[WiFiRecvOutlNum].serial->write((unsigned char)c);
		}

		WiFiData[i] = c;
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
				WiFiData[i + 1] = 0;
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
	digitalWrite(wrb2sakura(WIFI_CTS), 0);	//送信許可
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

	rbserial[WIFI_SERIAL].serial->print("AT+CWMODE=");
	rbserial[WIFI_SERIAL].serial->println(mode);

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
		rbserial[WIFI_SERIAL].serial->print("AT+");
	}

	for(int i=0; i<254; i++){
		if( i >= len){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[len] = 0;

	rbserial[WIFI_SERIAL].serial->println((const char*)WiFiData);
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

	rbserial[WIFI_SERIAL].serial->print("AT+CWJAP=");
	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	rbserial[WIFI_SERIAL].serial->print((const char*)WiFiData);

	for(int i=0; i<254; i++){
		if( i >= slen){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[slen] = 0;
	rbserial[WIFI_SERIAL].serial->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0x2C;		//-> ,
	WiFiData[2] = 0x22;		//-> "
	WiFiData[3] = 0;
	rbserial[WIFI_SERIAL].serial->print((const char*)WiFiData);

	for(int i=0; i<254; i++){
		if( i >= plen){ break; }
		WiFiData[i] = p[i];
	}
	WiFiData[plen] = 0;
	rbserial[WIFI_SERIAL].serial->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	rbserial[WIFI_SERIAL].serial->println((const char*)WiFiData);

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
	rbserial[WIFI_SERIAL].serial->println("AT+CIFSR");

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
	sci_str *Sci0 = Serial.get_handle();

	while(true){
		len0 = sci_rxcount_ex(Sci0);
		len1 = sci_rxcount_ex(rbserial[WIFI_SERIAL].sci);

		if(len0 > 0){
			len = len0<256 ? len0 : 256;

			for(int i=0; i<len; i++){
				WiFiData[i] = (unsigned char)Serial.read();
			}	        
			rbserial[WIFI_SERIAL].serial->write( WiFiData, len );
		}

		if(len1 > 0){
			len = len1<256 ? len1 : 256;
			
			for(int i=0; i<len; i++){
				WiFiData[i] = (unsigned char)rbserial[WIFI_SERIAL].serial->read();
			}
	        Serial.write( WiFiData, len );
		}
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}


//**************************************************
// ライブラリを定義します
//**************************************************
void esp8266_Init(mrb_state *mrb)
{	
	//ESP8266からの受信を出力しないに設定
	WiFiRecvOutlNum = -1;

	//CTS用にPIN15をOUTPUTに設定します
	pinMode(wrb2sakura(WIFI_CTS), 1);
	digitalWrite(wrb2sakura(WIFI_CTS), 1);

	//WiFiのシリアル3を設定
	//シリアル通信の初期化をします
	if(rbserial[WIFI_SERIAL].enable){
		rbserial[WIFI_SERIAL].serial->end();
		delay(50);
		delete rbserial[WIFI_SERIAL].serial;
	}
	rbserial[WIFI_SERIAL].serial = new CSerial();
	rbserial[WIFI_SERIAL].serial->begin(WIFI_BAUDRATE, SCI_SCI6B);
	rbserial[WIFI_SERIAL].enable = true;
	rbserial[WIFI_SERIAL].sci = rbserial[WIFI_SERIAL].serial->get_handle();

	sci_convert_crlf_ex(rbserial[WIFI_SERIAL].sci, CRLF_NONE, CRLF_NONE);		//バイナリを通せるようにする


	//ECHOオフコマンドを送信する
	rbserial[WIFI_SERIAL].serial->println("ATE0");

	getData(500);	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読む


	struct RClass *wifiModule = mrb_define_module(mrb, "WiFi");

	mrb_define_module_function(mrb, wifiModule, "at", mrb_wifi_at, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "sout", mrb_wifi_Sout, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "cwmode", mrb_wifi_Cwmode, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, wifiModule, "cwjap", mrb_wifi_Cwjap, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, wifiModule, "cifsr", mrb_wifi_Cifsr, MRB_ARGS_NONE());

	mrb_define_module_function(mrb, wifiModule, "bypass", mrb_wifi_bypass, MRB_ARGS_NONE());

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
