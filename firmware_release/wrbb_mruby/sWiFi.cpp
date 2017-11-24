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
#include <SD.h>

#include <mruby.h>
#include <mruby/string.h>
#include <mruby/array.h>

#include "../wrbb.h"
#include "sKernel.h"
#include "sSerial.h"

#include "sWiFi.h"

#if BOARD == BOARD_GR || FIRMWARE == SDBT || FIRMWARE == SDWF || BOARD == BOARD_P05 || BOARD == BOARD_P06
	#include "sSdCard.h"
#endif

extern HardwareSerial *RbSerial[];		//0:Serial(USB), 1:Serial1, 2:Serial3, 3:Serial2, 4:Serial6 5:Serial7

#define  WIFI_SERIAL	3
#define  WIFI_BAUDRATE	115200
//#define  WIFI_CTS		15
#define  WIFI_WAIT_MSEC	10000

unsigned char WiFiData[256];
int WiFiRecvOutlNum = -1;	//ESP8266からの受信を出力するシリアル番号: -1の場合は出力しない。

//#define	DEBUG		// Define if you want to debug
#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif


//**************************************************
// OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、
// 指定されたシリアルポートに出力します
//
// 1:受信した, 0:受信できなかった 2:受信がオーバーフローした
//**************************************************
int getData(unsigned int wait_msec)
{
unsigned long times;
int c;
int okt = 0;
int ert = 0;
int len = 0;
int n = 0;

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
			//DEBUG_PRINT("len=",len);
			//DEBUG_PRINT("n=",n);

			for(int i=0; i<len; i++){
				c = RbSerial[WIFI_SERIAL]->read();

				//指定のシリアルポートに出す設定であれば、受信値を出力します
				if(WiFiRecvOutlNum >= 0){
					RbSerial[WiFiRecvOutlNum]->write((unsigned char)c);
				}
				//DEBUG_PRINT("c=",c);

				WiFiData[n] = c;
				n++;

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

						// OK 0d0a || ERROR 0d0a
						WiFiData[n] = 0;
						return 1;
						//n = 256;
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

	//whileを抜けてくるということは、オーバーフローしている
	return 2;
}

//**************************************************
// ステーションモードの設定: WiFi.cwmode
//  WiFi.cwmode(mode)
//  WiFi.setMode(mode)
//  mode: 1:Station, 2:SoftAP, 3:Station + SoftAP
//**************************************************
mrb_value mrb_wifi_Cwmode(mrb_state *mrb, mrb_value self)
{
int	mode;

	mrb_get_args(mrb, "i", &mode);

	RbSerial[WIFI_SERIAL]->print("AT+CWMODE=");
	RbSerial[WIFI_SERIAL]->println(mode);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// コマンド応答のシリアル出力設定: WiFi.serialOut
//  WiFi.serialOut( mode[,serialNumber] )
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
		if(n >= 2){
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
//	commnad: ATコマンド文字列
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

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// WiFi接続します: WiFi.connect
//  WiFi.connect(SSID,Passwd)
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

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}


//**************************************************
// WiFiアクセスポイントになります: WiFi.softAP
//  WiFi.softAP(SSID,Passwd,Channel,Encrypt)
//  SSID: WiFiのSSID
//  Passwd: パスワード
//  Channel: チャネル
//  Encrypt: 暗号タイプ 0:Open, 1:WEP, 2:WPA_PSK, 3:WPA2_PSK, 4:WPA_WPA2_PSK
//**************************************************
mrb_value mrb_wifi_softAP(mrb_state *mrb, mrb_value self)
{
mrb_value ssid;
mrb_value passwd;
int ch = 1;
int enc = 0;

	mrb_get_args(mrb, "SSii", &ssid, &passwd, &ch, &enc);

	char *s = RSTRING_PTR(ssid);
	int slen = RSTRING_LEN(ssid);

	char *p = RSTRING_PTR(passwd);
	int plen = RSTRING_LEN(passwd);

	if (enc < 0 || enc>4){
		enc = 0;
	}

	RbSerial[WIFI_SERIAL]->print("AT+CWSAP=");
	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for (int i = 0; i<254; i++){
		if (i >= slen){ break; }
		WiFiData[i] = s[i];
	}
	WiFiData[slen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0x2C;		//-> ,
	WiFiData[2] = 0x22;		//-> "
	WiFiData[3] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	for (int i = 0; i<254; i++){
		if (i >= plen){ break; }
		WiFiData[i] = p[i];
	}
	WiFiData[plen] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	WiFiData[0] = 0x22;		//-> "
	WiFiData[1] = 0x2C;		//-> ,
	WiFiData[2] = 0;
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);

	RbSerial[WIFI_SERIAL]->print(ch);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(enc);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// アクセスポイントに接続されているIP取得: WiFi.connetedIP
//  WiFi.connectedIP()
//**************************************************
mrb_value mrb_wifi_connectedIP(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+CWLIF");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// DHCP有無の切り替え: WiFi.dhcp
//  WiFi.dhcp(mode, bool)
//  mode: 0:SoftAP, 1:Station, 2:Both softAP + Station
//  bool: 0:disable , 1:enable
//**************************************************
mrb_value mrb_wifi_dhcp(mrb_state *mrb, mrb_value self)
{
int	mode;
int bl = 0;

	mrb_get_args(mrb, "ii", &mode, &bl);

	RbSerial[WIFI_SERIAL]->print("AT+CWDHCP=");
	RbSerial[WIFI_SERIAL]->print(mode);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(bl);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// IPアドレスとMACアドレスの表示: WiFi.ipconfig
//  WiFi.ipconfig()
//  WiFi.cifsr()
//**************************************************
mrb_value mrb_wifi_Cifsr(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+CIFSR");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
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
	//int retCnt = 0;

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
// バージョンを取得します: WiFi.version
//  WiFi.version()
//**************************************************
mrb_value mrb_wifi_Version(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+GMR");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// WiFiを切断します: WiFi.disconnect
//  WiFi.disconnect()
//**************************************************
mrb_value mrb_wifi_Disconnect(mrb_state *mrb, mrb_value self)
{
	RbSerial[WIFI_SERIAL]->println("AT+CWQAP");

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// 複数接続可能モードの設定: WiFi.multiConnect
//  WiFi.multiConnect(mode)
//  mode: 0:1接続のみ, 1:4接続まで可能
//**************************************************
mrb_value mrb_wifi_multiConnect(mrb_state *mrb, mrb_value self)
{
int	mode;

	mrb_get_args(mrb, "i", &mode);

	RbSerial[WIFI_SERIAL]->print("AT+CIPMUX=");
	RbSerial[WIFI_SERIAL]->println(mode);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// ファイルに含まれる+IPDデータを削除します
// ipd: ipdデータ列
// strFname1: 元ファイル
// strFname2: 削除したファイル
//**************************************************
int CutGarbageData(const char *ipd, const char *strFname1, const char *strFname2)
{
File fp, fd;

#if BOARD == BOARD_GR
	int led = digitalRead(PIN_LED0) | ( digitalRead(PIN_LED1)<<1) | (digitalRead(PIN_LED2)<<2)| (digitalRead(PIN_LED3)<<3);
#else
	int led = digitalRead(RB_LED);
#endif

	if( SD.exists(strFname2)){
		SD.remove(strFname2);
	}

	if( !(fp = SD.open(strFname1, FILE_READ)) ){
		return 2;
	}

	if( !(fd = SD.open(strFname2, FILE_WRITE)) ){
		return 3;
	}

	int tLED = 0;
	int ipdLen = strlen(ipd);
	int cnt;
	unsigned char c;
	int rc;
	bool findFlg = true;
	unsigned char str[16];
	int dLen;
	unsigned long seekCnt = 0;
	while(true){

		//LEDを点滅させる
#if BOARD == BOARD_GR
		digitalWrite(PIN_LED0, tLED);
		digitalWrite(PIN_LED1, tLED);
		digitalWrite(PIN_LED2, tLED);
		digitalWrite(PIN_LED3, tLED);
#else
		digitalWrite(RB_LED, tLED);
#endif
		tLED = 1 - tLED;

		//+IPD文字列を探します
		cnt = 0;
		while(true){
			rc = fp.read();

			if(rc < 0){
				findFlg = false;
				break;
			}

			c = (unsigned char)rc;
			if(ipd[cnt] == c){
				cnt++;
				if(cnt == ipdLen){
					seekCnt += cnt;
					break;
				}
			}
			else if(c == 0x0D){
				cnt = 1;
			}
			else{
				cnt = 0;
			}
		}

		//Serial.print("findFlg= ");
		//Serial.println(findFlg);

		if(findFlg == false){	break;	}

		//ここから後はバイト数が来ているはず
		cnt = 0;
		while(true){

			rc = fp.read();

			if(rc < 0){
				findFlg = false;
				break;
			}
			c = (unsigned char)rc;

			str[cnt] = c;

			if(c == ':'){
				str[cnt] = 0;
				seekCnt += cnt + 1;
				break;
			}
			else if(cnt >= 15){
				str[15] = 0;
				findFlg = false;
				break;
			}
			cnt++;
		}

		if(findFlg == false){	break;	}
	
		//読み込むバイト数を求めます
		dLen = atoi((const char*)str);

		seekCnt += dLen;

		//Serial.print("dLen= ");
		//Serial.println((const char*)str);

		while(dLen > 0){
			if(dLen >= 256){
				 fp.read(WiFiData, 256);

				 fd.write( WiFiData, 256);
				 dLen -= 256;
			}
			else{
				 fp.read(WiFiData, dLen);
				 fd.write( WiFiData, dLen);

				 dLen = 0;
			}
		}
	}

	if(findFlg == false){
		//処理していないところは、そのまま書きます
		fp.seek(seekCnt);

		while(true){
			dLen = fp.read(WiFiData, 256);
			fd.write(WiFiData, dLen);
			if(dLen < 256){
				break;
			}
		}
	}

	fd.flush();
	fd.close();

	fp.close();

	//LEDを元の状態に戻す
#if BOARD == BOARD_GR
	digitalWrite(PIN_LED0, led & 1);
	digitalWrite(PIN_LED1, (led >> 1) & 1);
	digitalWrite(PIN_LED2, (led >> 2) & 1);
	digitalWrite(PIN_LED3, (led >> 3) & 1);
#else
	digitalWrite(RB_LED, led);
#endif

	return 1;
}

//**************************************************
// http GETをSDカードに保存します: WiFi.httpGetSD
//  WiFi.httpGetSD( Filename, URL[,Headers] )
//	Filename: 保存するファイル名
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//		2: SDカードが使えない
//		... 各種エラー
//**************************************************
mrb_value mrb_wifi_getSD(mrb_state *mrb, mrb_value self)
{
mrb_value vFname, vURL, vHeaders;
const char *tmpFilename = "wifitmp.tmp";
const char *hedFilename = "hedrfile.tmp";
char	*strFname, *strURL;
int len = 0;
File fp, fd;
int sla, koron;

	//SDカードが利用可能か確かめます
	if(!sdcard_Init(mrb)){
		return mrb_fixnum_value( 2 );
	}

	int n = mrb_get_args(mrb, "SS|A", &vFname, &vURL, &vHeaders);

	strFname = RSTRING_PTR(vFname);
	strURL = RSTRING_PTR(vURL);

	//httpサーバに送信するデータを、strFname ファイルに生成します。

	//既にファイルがあれば消す
	if( SD.exists(hedFilename)){
		SD.remove(hedFilename);
	}
	//ファイルオープン
	if( !(fp = SD.open(hedFilename, FILE_WRITE)) ){
		return mrb_fixnum_value( 3 );
	}

	//1行目を生成
	{
		fp.write( (unsigned char*)"GET /", 5);

		//URLからドメインを分割する
		len = strlen(strURL);
		sla = len;
		koron = 0;
		for(int i=0; i<len; i++){
			if(strURL[i] == '/'){
				sla = i;
				break;
			}
			if(strURL[i] == ':'){
				koron = i;
			}
		}

		for(int i=sla + 1; i<len; i++){
			fp.write(strURL[i]);
		}

		fp.write( (unsigned char*)" HTTP/1.1", 9);
		fp.write(0x0D);	fp.write(0x0A);
	}

	//Hostヘッダを生成
	{
		fp.write( (unsigned char*)"Host: ", 6);
	
		if(koron == 0){
			koron = sla;
		}

		for(int i=0; i<koron; i++){
			fp.write(strURL[i]);
		}
		fp.write(0x0D);	fp.write(0x0A);
	}

	//ヘッダ情報が追加されているとき
	if(n >= 3){
		n = RARRAY_LEN( vHeaders );
		mrb_value hes;
		for (int i=0; i<n; i++) {
	
			hes = mrb_ary_ref(mrb, vHeaders, i);
			len = strlen(RSTRING_PTR(hes));
			
			//ヘッダの追記
			fp.write(RSTRING_PTR(hes), len);
			fp.write(0x0D);	fp.write(0x0A);
		}
	}

	//改行のみの行を追加する
	fp.write(0x0D);	fp.write(0x0A);

	fp.flush();
	fp.close();


	//****** AT+CIPSTARTコマンド ******

	//WiFiData[]に、ドメインとポート番号を取得
	for(int i=0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if(i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if( koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		RbSerial[WIFI_SERIAL]->println("80");
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return mrb_fixnum_value( 0 );
	}
	//Serial.print("httpServer Connect: ");
	//Serial.print((const char*)WiFiData);

	//****** AT+CIPSEND コマンド ******

	//送信データサイズ取得
	if( !(fp = SD.open(hedFilename, FILE_READ)) ){
		return mrb_fixnum_value( 4 );
	}
	//ファイルサイズ取得
	int sByte = fp.size();
	fp.close();

	//Serial.print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");

	sprintf((char*)WiFiData, "%d", sByte);

	//Serial.println((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return mrb_fixnum_value( 0 );
	}

	//Serial.print("> Waiting: ");
	//Serial.print((const char*)WiFiData);

	//****** 送信データ受付モードになったので、http GETデータを送信する ******
	{
		if( !(fp = SD.open(hedFilename, FILE_READ)) ){
			return mrb_fixnum_value( 5 );
		}
		WiFiData[1] = 0;
		for(int i=0; i<sByte; i++){
			WiFiData[0] = (unsigned char)fp.read();
			//Serial.print((const char*)WiFiData);
			RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
		}
		fp.close();

		SD.remove(tmpFilename);		//受信するためファイルを事前に消している

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
			return mrb_fixnum_value( 0 );
		}
		//Serial.print("Send Finish: ");
		//Serial.print((const char*)WiFiData);
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	if( !(fp = SD.open(tmpFilename, FILE_WRITE)) ){
		return mrb_fixnum_value( 6 );
	}

	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	unsigned char recv[2];
	times = millis();

#if BOARD == BOARD_GR
	int led = digitalRead(PIN_LED0) | ( digitalRead(PIN_LED1)<<1) | (digitalRead(PIN_LED2)<<2)| (digitalRead(PIN_LED3)<<3);
#else
	int led = digitalRead(RB_LED);
#endif

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while(len = RbSerial[WIFI_SERIAL]->available())
		{
			//LEDを点灯する
#if BOARD == BOARD_GR
			digitalWrite(PIN_LED0, HIGH);
			digitalWrite(PIN_LED1, HIGH);
			digitalWrite(PIN_LED2, HIGH);
			digitalWrite(PIN_LED3, HIGH);
#else
			digitalWrite(RB_LED, HIGH);
#endif
			for(int i=0; i<len; i++){
				recv[0] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
				fp.write( (unsigned char*)recv, 1);
			}
			times = millis();
			wait_msec = 1000;	//データが届き始めたら、1sec待ちに変更する

			//LEDを消灯する
#if BOARD == BOARD_GR
			digitalWrite(PIN_LED0, LOW);
			digitalWrite(PIN_LED1, LOW);
			digitalWrite(PIN_LED2, LOW);
			digitalWrite(PIN_LED3, LOW);
#else
			digitalWrite(RB_LED, LOW);
#endif
		}
	}
	fp.flush();
	fp.close();

	//****** 受信終了 ******
	//Serial.println("Recv Finish");

	//受信データに '\r\n+\r\n+IPD,4,****:'というデータがあるので削除します
	int ret = CutGarbageData("\r\n+IPD,4,", tmpFilename, strFname);
	if(ret != 1){
		return mrb_fixnum_value( 7 );
	}

	//****** AT+CIPCLOSE コマンド ******
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);
	
	//Serial.println((const char*)WiFiData);

#if BOARD == BOARD_GR
	digitalWrite(PIN_LED0, led & 1);
	digitalWrite(PIN_LED1, (led >> 1) & 1);
	digitalWrite(PIN_LED2, (led >> 2) & 1);
	digitalWrite(PIN_LED3, (led >> 3) & 1);
#else
	digitalWrite(RB_LED, led);
#endif

	return mrb_fixnum_value( 1 );
}

//**************************************************
// http GETプロトコルを送信する: WiFi.httpGet
//  WiFi.httpGet( URL[,Headers] )
//　送信のみで、結果を受信しない　
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//**************************************************
mrb_value mrb_wifi_get(mrb_state *mrb, mrb_value self)
{
mrb_value vURL, vHeaders;
char	*strURL;
int len = 0;
int sla, cnt;
int koron = 0;
char sData[1024];

	int n = mrb_get_args(mrb, "S|A", &vURL, &vHeaders);

	strURL = RSTRING_PTR(vURL);

	//URLからドメインを分割する
	len = strlen(strURL);
	sla = len;
	for(int i=0; i<len; i++){
		if(strURL[i] == '/'){
			sla = i;
			break;
		}
		if(strURL[i] == ':'){
			koron = i;
		}
	}

	if(koron == 0){
		koron = sla;
	}

	//****** AT+CIPSTARTコマンド ******
	//WiFiData[]に、ドメインとポート番号を取得
	for(int i=0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if(i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if( koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		RbSerial[WIFI_SERIAL]->println("80");
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return mrb_fixnum_value( 0 );
	}

	//****** AT+CIPSEND コマンド ******

	//ヘッダの1行目を生成
	{
		strcpy(sData, "GET /");

		cnt = 5;
		for(int i=sla + 1; i<len; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}

		sData[cnt] = 0;
		strcat(sData, " HTTP/1.1\r\n");
	}

	//Hostヘッダを生成
	{
		strcat(sData, "Host: ");
	
		cnt = strlen(sData);
		for(int i=0; i<koron; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}
		sData[cnt] = 0;
		strcat(sData, "\r\n");
	}

	//ヘッダ情報が追加されているとき
	if(n >= 2){
		n = RARRAY_LEN( vHeaders );
		mrb_value hes;
		for (int i=0; i<n; i++) {
	
			hes = mrb_ary_ref(mrb, vHeaders, i);
			len = strlen(RSTRING_PTR(hes));
			
			//ヘッダの追記
			strcat(sData, RSTRING_PTR(hes));
			strcat(sData, "\r\n");
		}
	}

	//改行のみの行を追加する
	strcat(sData, "\r\n");

	//送信データサイズ取得
	len = strlen(sData);

	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->println(len);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return mrb_fixnum_value( 0 );
	}

	//****** 送信データ受付モードになったので、http GETデータを送信する ******
	{
		RbSerial[WIFI_SERIAL]->print((const char*)sData);

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
			return mrb_fixnum_value( 0 );
		}
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	times = millis();

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while(len = RbSerial[WIFI_SERIAL]->available())
		{
			for(int i=0; i<len; i++){
				RbSerial[WIFI_SERIAL]->read();
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}
	//****** 受信終了 ******

	//****** AT+CIPCLOSE コマンド ******
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);

	return mrb_fixnum_value( 1 );
}

//**************************************************
// TCP/UDP接続を閉じる: WiFi.cClose
//  WiFi.cClose(number)
//  number: 接続番号(1～4)
//**************************************************
mrb_value mrb_wifi_cClose(mrb_state *mrb, mrb_value self)
{
int	num;

	mrb_get_args(mrb, "i", &num);

	RbSerial[WIFI_SERIAL]->print("AT+CIPCLOSE=");
	RbSerial[WIFI_SERIAL]->println(num);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// UDP接続を開始します: WiFi.udpOpen
//  WiFi.udpOpen( number, IP_Address, SendPort, ReceivePort )
//　number: 接続番号(1～4) 
//	IP_Address: 通信相手アドレス
//	SendPort: 送信ポート番号
//	ReceivePort: 受信ポート番号
//**************************************************
mrb_value mrb_wifi_udpOpen(mrb_state *mrb, mrb_value self)
{
mrb_value vIpAdd;
char	*strIpAdd;
int	num, sport, rport;

	mrb_get_args(mrb, "iSii", &num, &vIpAdd, &sport, &rport);
	strIpAdd = RSTRING_PTR(vIpAdd);

	//****** AT+CIPSTARTコマンド ******
	RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=");
	RbSerial[WIFI_SERIAL]->print(num);
	RbSerial[WIFI_SERIAL]->print(",\"UDP\",\"");
	RbSerial[WIFI_SERIAL]->print((const char*)strIpAdd);
	RbSerial[WIFI_SERIAL]->print("\",");
	RbSerial[WIFI_SERIAL]->print(sport);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(rport);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// 指定接続番号にデータを送信します: WiFi.send
//  WiFi.send( number, Data[, length] )
//　number: 接続番号(0～3) 
//	Data: 送信するデータ
//　length: 送信データサイズ
//
//  戻り値は
//	  送信データサイズ
//**************************************************
mrb_value mrb_wifi_send(mrb_state *mrb, mrb_value self)
{
mrb_value vdata;
char	*strdata;
int	num, len;

	int n = mrb_get_args(mrb, "iS|i", &num, &vdata, &len);
	strdata = RSTRING_PTR(vdata);

	//送信データサイズが指定されていないとき
	if(n < 3){
		len = RSTRING_LEN(vdata);
	}

	//****** AT+CIPSTARTコマンド ******
	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=");
	RbSerial[WIFI_SERIAL]->print(num);
	RbSerial[WIFI_SERIAL]->print(",");
	RbSerial[WIFI_SERIAL]->println(len);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return mrb_fixnum_value( 0 );
	}

	RbSerial[WIFI_SERIAL]->print((const char*)strdata);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){

		//タイムアウトと思われるので、強制的にデータサイズの不足分の0x0Dを送信する
		for(int i=0; i<len-strlen(strdata); i++){
			RbSerial[WIFI_SERIAL]->print("\r");
		}
		return mrb_fixnum_value( 0 );
	}

	return mrb_fixnum_value( len );
}

//**************************************************
// 指定接続番号からデータを受信します: WiFi.recv
//  WiFi.recv( number )
//　number: 接続番号(0～3) 
//
//  戻り値は
//	  受信したデータの配列　ただし、256以下
//**************************************************
mrb_value mrb_wifi_recv(mrb_state *mrb, mrb_value self)
{
int	num;
unsigned char str[16];
mrb_value arv[256];

	mrb_get_args(mrb, "i", &num);

	sprintf((char*)str, "\r\n+IPD,%d,", num);

	//Serial.println((const char*)str);

	if(RbSerial[WIFI_SERIAL]->available() == 0){
		arv[0] = mrb_fixnum_value(-1);
		return mrb_ary_new_from_values(mrb, 1, arv);
	}

	//****** 受信開始 ******
	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	times = millis();
	int len = strlen((char*)str);
	int cnt = 0;
	unsigned char c;

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		if(RbSerial[WIFI_SERIAL]->available())
		{
			c = (unsigned char)RbSerial[WIFI_SERIAL]->read();

			if(str[cnt] == c){
				cnt++;
				if(cnt == len){
					break;
				}
			}
			else if(c == 0x0D){
				cnt = 1;
			}
			else{
				cnt = 0;
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}

	//ここから後はバイト数が来ているはず
	times = millis();
	cnt = 0;
	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			str[cnt] = 0;
			break;
		}

		if(RbSerial[WIFI_SERIAL]->available())
		{
			c = (unsigned char)RbSerial[WIFI_SERIAL]->read();

			str[cnt] = c;
			if(c == ':'){
				str[cnt] = 0;
				break;
			}
			else if(cnt >= 15){
				str[15] = 0;
				break;
			}
			cnt++;
			times = millis();
		}
	}

	len = atoi((const char*)str);

	//Serial.print("len= ");
	//Serial.println(len);

	//データを取りだします
	times = millis();
	cnt = 0;
	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		if(RbSerial[WIFI_SERIAL]->available())
		{
			arv[cnt] = mrb_fixnum_value(RbSerial[WIFI_SERIAL]->read());
			cnt++;

			if(cnt >= len){
				break;
			}
			times = millis();
		}
	}
	//****** 受信終了 ******

	return mrb_ary_new_from_values(mrb, cnt, arv);
}

//**************************************************
// http POSTとしてSDカードのファイルをPOSTします: WiFi.httpPostSD
//  WiFi.httpPostSD( URL, Headers, Filename )
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//	Filename: POSTするファイル名
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//		2: SDカードが使えない
//		... 各種エラー
//**************************************************
mrb_value mrb_wifi_postSD(mrb_state *mrb, mrb_value self)
{
mrb_value vFname, vURL, vHeaders;
const char *headFilename = "headfile.tmp";
char	*strFname, *strURL;
int len = 0;
File fp, fd;
int sla, koron;
int sBody, sHeader;

	//SDカードが利用可能か確かめます
	if (!sdcard_Init(mrb)){
		return mrb_fixnum_value(2);
	}

	int n = mrb_get_args(mrb, "SAS", &vURL, &vHeaders, &vFname );

	strFname = RSTRING_PTR(vFname);
	strURL = RSTRING_PTR(vURL);

	//送信ファイルサイズ取得
	if (!(fp = SD.open(strFname, FILE_READ))){
		return mrb_fixnum_value(3);
	}
	//ファイルサイズ取得
	sBody = fp.size();
	fp.close();

	//httpサーバに送信するデータを、strFname ファイルに生成します。

	//既にファイルがあれば消す
	if (SD.exists(headFilename)){
		SD.remove(headFilename);
	}
	//ファイルオープン
	if (!(fp = SD.open(headFilename, FILE_WRITE))){
		return mrb_fixnum_value(4);
	}

	//1行目を生成
	{
		fp.write((unsigned char*)"POST /", 6);

		//URLからドメインを分割する
		len = strlen(strURL);
		sla = len;
		koron = 0;
		for (int i = 0; i<len; i++){
			if (strURL[i] == '/'){
				sla = i;
				break;
			}
			if (strURL[i] == ':'){
				koron = i;
			}
		}

		for (int i = sla + 1; i<len; i++){
			fp.write(strURL[i]);
		}

		fp.write((unsigned char*)" HTTP/1.1", 9);
		fp.write(0x0D);	fp.write(0x0A);
	}

	//Hostヘッダを生成
	{
		fp.write((unsigned char*)"Host: ", 6);

		if (koron == 0){
			koron = sla;
		}

		for (int i = 0; i < koron; i++){
			fp.write(strURL[i]);
		}
		fp.write(0x0D);	fp.write(0x0A);
	}

	//Content-Lengthを付けます
	{
		fp.write((unsigned char*)"Content-Length: ", 16);
		sprintf((char*)WiFiData,"%u", sBody);
		fp.write(WiFiData,strlen((char*)WiFiData));
		fp.write(0x0D);	fp.write(0x0A);
	}

	//ヘッダ情報が追加されているとき
	{
		n = RARRAY_LEN(vHeaders);
		mrb_value hes;
		for (int i = 0; i<n; i++) {

			hes = mrb_ary_ref(mrb, vHeaders, i);
			len = strlen(RSTRING_PTR(hes));

			//ヘッダの追記
			fp.write(RSTRING_PTR(hes), len);
			fp.write(0x0D);	fp.write(0x0A);
		}
	}

	//改行のみの行を追加する
	fp.write(0x0D);	fp.write(0x0A);

	fp.flush();
	fp.close();


	//****** AT+CIPSTARTコマンド ******

	//WiFiData[]に、ドメインとポート番号を取得
	for (int i = 0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if (i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if (koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		RbSerial[WIFI_SERIAL]->println("80");
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if (!(WiFiData[strlen((const char*)WiFiData) - 2] == 'K' || WiFiData[strlen((const char*)WiFiData) - 3] == 'K')){
		return mrb_fixnum_value(0);
	}
	//Serial.print("httpServer Connect: ");
	//Serial.print((const char*)WiFiData);

	//****** AT+CIPSEND コマンド ******
	
	//送信ヘッダのサイズ取得
	if (!(fp = SD.open(headFilename, FILE_READ))){
		return mrb_fixnum_value(5);
	}
	//ファイルサイズ取得
	sHeader = fp.size();
	fp.close();

	//Serial.print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");

	sprintf((char*)WiFiData, "%u", sHeader + sBody);

	//Serial.println((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->println((const char*)WiFiData);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if (!(WiFiData[strlen((const char*)WiFiData) - 2] == 'K' || WiFiData[strlen((const char*)WiFiData) - 3] == 'K')){
		return mrb_fixnum_value(0);
	}

	//Serial.print("> Waiting: ");
	//Serial.print((const char*)WiFiData);

	//****** 送信データ受付モードになったので、http POSTデータを送信する ******
	{
		//先ずヘッダデータを送信する
		if (!(fp = SD.open(headFilename, FILE_READ))){
			return mrb_fixnum_value(6);
		}
		WiFiData[1] = 0;
		for (int i = 0; i<sHeader; i++){
			WiFiData[0] = (unsigned char)fp.read();
			//Serial.print((const char*)WiFiData);
			RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
		}
		fp.close();

		//先ずボディデータを送信する
		if (!(fp = SD.open(strFname, FILE_READ))){
			return mrb_fixnum_value(7);
		}
		WiFiData[1] = 0;
		for (int i = 0; i<sBody; i++){
			WiFiData[0] = (unsigned char)fp.read();
			//Serial.print((const char*)WiFiData);
			RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
		}
		fp.close();

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if (!(WiFiData[strlen((const char*)WiFiData) - 2] == 'K' || WiFiData[strlen((const char*)WiFiData) - 3] == 'K')){
			return mrb_fixnum_value(0);
		}
		//Serial.print("Send Finish: ");
		//Serial.print((const char*)WiFiData);
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	times = millis();

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while(len = RbSerial[WIFI_SERIAL]->available())
		{
			for(int i=0; i<len; i++){
				RbSerial[WIFI_SERIAL]->read();
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}
	//****** 受信終了 ******

	//****** AT+CIPCLOSE コマンド ******
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);

	return mrb_fixnum_value( 1 );
}

//**************************************************
// http POSTする: WiFi.httpPost
//  WiFi.httpPost( URL, Headers, data )
//　送信のみで、結果を受信しない
//	URL: URL
//	Headers: ヘッダに追記する文字列の配列
//　Data: POSTデータ
//
//  戻り値は以下のとおり
//		0: 失敗
//		1: 成功
//**************************************************
mrb_value mrb_wifi_post(mrb_state *mrb, mrb_value self)
{
mrb_value vData, vURL, vHeaders;
char	*strData;
char	*strURL;
int		sBody, sHeader;
int sla, cnt;
int koron = 0;
char sData[1024];
int len;

	mrb_get_args(mrb, "SAS", &vURL, &vHeaders, &vData);

	strData = RSTRING_PTR(vData);
	sBody = strlen(strData);

	strURL = RSTRING_PTR(vURL);

	//URLからドメインを分割する
	len = strlen(strURL);
	sla = len;
	for(int i=0; i<len; i++){
		if(strURL[i] == '/'){
			sla = i;
			break;
		}
		if(strURL[i] == ':'){
			koron = i;
		}
	}

	if(koron == 0){
		koron = sla;
	}

	//****** AT+CIPSTARTコマンド ******
	//WiFiData[]に、ドメインとポート番号を取得
	for(int i=0; i<sla; i++){
		WiFiData[i] = strURL[i];
		if(i == koron){
			WiFiData[i] = 0;
		}
	}
	WiFiData[sla] = 0;

	RbSerial[WIFI_SERIAL]->print("AT+CIPSTART=4,\"TCP\",\"");
	RbSerial[WIFI_SERIAL]->print((const char*)WiFiData);
	RbSerial[WIFI_SERIAL]->print("\",");
	if( koron < sla){
		RbSerial[WIFI_SERIAL]->println((const char*)&WiFiData[koron + 1]);
	}
	else{
		RbSerial[WIFI_SERIAL]->println("80");
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);

	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return mrb_fixnum_value( 0 );
	}

	//****** AT+CIPSEND コマンド ******

	//ヘッダの1行目を生成
	{
		strcpy(sData, "POST /");

		cnt = 6;
		for(int i=sla + 1; i<len; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}

		sData[cnt] = 0;
		strcat(sData, " HTTP/1.1\r\n");
	}

	//Hostヘッダを生成
	{
		strcat(sData, "Host: ");
	
		cnt = strlen(sData);
		for(int i=0; i<koron; i++){
			sData[cnt] = strURL[i];
			cnt++;
		}
		sData[cnt] = 0;
		strcat(sData, "\r\n");
	}

	//Content-Lengthを付けます
	{
		strcat(sData, "Content-Length: ");

		sprintf((char*)WiFiData,"%d", sBody);
		strcat(sData, (char*)WiFiData);
		strcat(sData, "\r\n");
	}

	//ヘッダ情報を付けます
	{
		int n = RARRAY_LEN( vHeaders );
		mrb_value hes;
		for (int i=0; i<n; i++) {
			hes = mrb_ary_ref(mrb, vHeaders, i);
			
			//ヘッダの追記
			strcat(sData, RSTRING_PTR(hes));
			strcat(sData, "\r\n");
		}
	}

	//改行のみの行を追加する
	strcat(sData, "\r\n");

	//送信データサイズ取得
	sHeader = strlen(sData);
	len = sHeader + sBody;


	RbSerial[WIFI_SERIAL]->print("AT+CIPSEND=4,");
	RbSerial[WIFI_SERIAL]->println(len);

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
		return mrb_fixnum_value( 0 );
	}

	//****** 送信データ受付モードになったので、http POSTデータを送信する ******
	{
		//ヘッダを送信する
		RbSerial[WIFI_SERIAL]->print((const char*)sData);
		//Serial.print(sData);

		//ボディを送信する
		RbSerial[WIFI_SERIAL]->print((const char*)strData);
		//Serial.print(strData);

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
		getData(WIFI_WAIT_MSEC);

		if( !(WiFiData[strlen((const char*)WiFiData)-2] == 'K' || WiFiData[strlen((const char*)WiFiData)-3] == 'K')){
			return mrb_fixnum_value( 0 );
		}
	}
	//****** 送信終了 ******

	//****** 受信開始 ******
	unsigned long times;
	unsigned int wait_msec = WIFI_WAIT_MSEC;
	times = millis();

	while(true){
		//wait_msec 待つ
		if(millis() - times > wait_msec){
			break;
		}

		while(len = RbSerial[WIFI_SERIAL]->available())
		{
			for(int i=0; i<len; i++){
				RbSerial[WIFI_SERIAL]->read();
			}
			times = millis();
			wait_msec = 100;	//データが届き始めたら、100ms待ちに変更する
		}
	}
	//****** 受信終了 ******

	//****** AT+CIPCLOSE コマンド ******
	RbSerial[WIFI_SERIAL]->println("AT+CIPCLOSE=4");
	getData(WIFI_WAIT_MSEC);

	return mrb_fixnum_value( 1 );
}

//**************************************************
// httpサーバを開始します: WiFi.httpServerSD
//  WiFi.httpServerSD( [Port] )
//　Port: 待ちうけポート番号
//　　　　-1: サーバ停止
//
//　SDカードが必須となります。
//　ポート番号を省略したときはアクセス確認します
//　戻り値
//　0: アクセスはありません
//　1: アクセスあり
//　2: SDカードが使えません
//　3: ファイルのアクセスに失敗しました
//
//　クライアントからアクセスがあるとき、通信内容と接続番号の2つが返ります
//  GET: パスが返ります
//  GET以外、ヘッダの1行目が返ります
//  ,
//  接続番号が返ります
//
//  revData, conNum = WiFi.httpServerSD()
//**************************************************
mrb_value mrb_wifi_serverSD(mrb_state *mrb, mrb_value self)
{
const char *tmpFilename = "header.tmp";
const char *headFilename = "header.txt";
char ipd[] = { 0x0d, 0x0a, '+', 'I', 'P', 'D', ',', '0', ',', 0x00 };
int len = 0;
File fp;
int	port = 80;
int sesnum = 0;
int headerSize = 0;
int rc, posi;
mrb_value arv[2];

	int n = mrb_get_args(mrb, "|i", &port);

	if(n < 1 ){
		if(RbSerial[WIFI_SERIAL]->available() > 0){

			SD.remove(tmpFilename);		//受信するためファイルを事前に消している

			if( !(fp = SD.open(tmpFilename, FILE_WRITE)) ){
				return mrb_fixnum_value( 3 );
			}

			unsigned long times;
			unsigned int wait_msec = WIFI_WAIT_MSEC;
			unsigned char recv[2];
			times = millis();

			while(true){
				//wait_msec 待つ
				if(millis() - times > wait_msec){
					break;
				}

				while(len = RbSerial[WIFI_SERIAL]->available())
				{
					for(int i=0; i<len; i++){
						recv[0] = (unsigned char)RbSerial[WIFI_SERIAL]->read();
						fp.write( (unsigned char*)recv, 1);
					}
					times = millis();
					wait_msec = 1000;	//データが届き始めたら、1sec待ちに変更する
				}
			}
			fp.flush();
			fp.close();
		
			//セッション番号を取得します
			{
				if (!(fp = SD.open(tmpFilename, FILE_READ))){
					return mrb_fixnum_value(3);
				}
				headerSize = fp.size();

				unsigned char rcvc;
				posi = 0;
				//WiFiData[]に先頭行を取得します
				for (int i = 0; i < headerSize; i++){
					rcvc = (unsigned char)fp.read();

					if (posi == 0 && rcvc == '+'){
						posi++;
					}
					else if (posi== 1 && rcvc == 'I'){
						posi++;
					}
					else if (posi == 2 && rcvc == 'P'){
						posi++;
					}
					else if (posi == 3 && rcvc == 'D'){
						posi++;
					}
					else if (posi == 4 && rcvc == ','){
						// "+IPD,"を見つけた
						break;
					}
					else{
						posi = 0;
					}
				}
				if (posi == 4){
					sesnum = fp.read() - 0x30;
				}
				fp.close();
			}

			//受信データに '\r\n+\r\n+IPD,0,****:'というデータがあるので削除します
			ipd[7] = sesnum + 0x30;
			int ret = CutGarbageData((const char*)ipd, tmpFilename, headFilename);
			if(ret != 1){
				return mrb_fixnum_value( 3 );
			}

			//***** GETならパスを返します ******
			unsigned char *uc;
			{
				if( !(fp = SD.open(headFilename, FILE_READ)) ){
					return mrb_fixnum_value( 3 );
				}
				headerSize = fp.size();

				//WiFiData[]に先頭行を取得します
				for(int i=0; i<headerSize; i++){
					rc = fp.read();
					WiFiData[i] = (unsigned char)rc;
					if(i >= 255){
						posi = i;
						break;
					}
					if(WiFiData[i] == 0x0A){
						posi = i + 1;
						break;
					}
				}
				WiFiData[posi] = 0;

				fp.close();
			}

			if( (posi < 6) || !(WiFiData[0] == 'G' && WiFiData[1] == 'E' && WiFiData[2] == 'T')){
				arv[0] = mrb_str_new_cstr(mrb, (const char*)WiFiData);
				arv[1] = mrb_fixnum_value(sesnum);
				return mrb_ary_new_from_values(mrb, 2, arv);
			}

			// 先頭の'/'を探します
			posi = -1;
			for(int i=0; i<headerSize; i++){
				if(WiFiData[i] == '/'){
					posi = i;
					break;
				}
			}

			if(posi == -1){
				arv[0] = mrb_str_new_cstr(mrb, (const char*)WiFiData);
				arv[1] = mrb_fixnum_value(sesnum);
				return mrb_ary_new_from_values(mrb, 2, arv);
			}

			//posi以降のスペースを探してnullを入れます
			for(int i=posi + 1; i<headerSize; i++){
				if(WiFiData[i] == ' '){
					WiFiData[i] = 0;
					break;
				}
			}
		
			uc = &WiFiData[posi];
			arv[0] = mrb_str_new_cstr(mrb, (const char*)uc);
			arv[1] = mrb_fixnum_value(sesnum);
			return mrb_ary_new_from_values(mrb, 2, arv);
		}
		else{
			//データ無し
			return mrb_fixnum_value( 0 );
		}
	}

	//SDカードが利用可能か確かめます
	if (!sdcard_Init(mrb)){
		return mrb_fixnum_value( 2 );
	}

	//****** AT+CIPSERVERコマンド ******
	if(port < 0){
		RbSerial[WIFI_SERIAL]->println("AT+CIPSERVER=0");
	}
	else{
		RbSerial[WIFI_SERIAL]->print("AT+CIPSERVER=1,");
		RbSerial[WIFI_SERIAL]->println(port);
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

//**************************************************
// httpサーバを開始します: WiFi.httpServer
//  WiFi.httpServer( [Port] )
//　Port: 待ちうけポート番号
//　　　　-1: サーバ停止
//
//　ポート番号を省略したときはアクセス確認します
//　戻り値
//　0: アクセスはありません
//　文字データ: アクセスあり
//
//　クライアントからアクセスがあるとき、通信内容と接続番号の2つが返ります
//  GET: パスが返ります
//  GET以外、取り込んだ200バイト程度が返ります
//  ,
//  接続番号が返ります
//
//  revData, conNum = WiFi.httpServer()
//**************************************************
mrb_value mrb_wifi_server(mrb_state *mrb, mrb_value self)
{
	char ipd[] = { 0x0d, 0x0a, '+', 'I', 'P', 'D', ',' };
	int	port = 80;
	int sesnum = 0;
	mrb_value arv[2];

	int n = mrb_get_args(mrb, "|i", &port);

	if (n < 1){
		if (RbSerial[WIFI_SERIAL]->available() > 0){
			unsigned char recv[256];
			unsigned char c;
			bool rwriteFlg = true;
			bool snmFlg = false;
			bool crnFlg = false;
			bool redFlg = true;		//検索文字列が途中で違ったときに、違った文字が検索文字の先頭文字と同じだった場合に、次の文字を読み込まずに、その文字から検索開始したいときにfalseにします。
			int ipdCnt = 0;
			int recCnt = 0;
			int wifCnt = 0;
			unsigned long times;
			unsigned int wait_msec = WIFI_WAIT_MSEC;
			times = millis();

			while (true){
				//wait_msec 待つ
				if (millis() - times > wait_msec){
					break;
				}

				while (RbSerial[WIFI_SERIAL]->available())
				{
					if (redFlg){
						c = (unsigned char)RbSerial[WIFI_SERIAL]->read();
					}

					if (snmFlg){
						sesnum = c - 0x30;
						//Serial.print("S Num = ");
						//Serial.println(sesnum);

						snmFlg = false;
						crnFlg = true;	//コロンを見つけるまで飛ばすフラグ
					}
					else if (crnFlg){
						if (c == ':'){
							crnFlg = false;	//コロンを見つけるまで読み飛ばす処理終わり
							ipdCnt = 0;		//\r\nIPD+,検索条件の初期化
							recCnt = -1;	//バッファしたデータの廃棄フラグ
						}
					}
					else if (c == ipd[ipdCnt]){
						//Serial.print("|");
						//Serial.print((int)ipd[ipdCnt]);
						//Serial.println("|");
						redFlg = true;
						rwriteFlg = true;
						ipdCnt++;
						if (ipdCnt == 7){
							//"\r\n+IPD,"を見つけた
							snmFlg = true;	//セッション番号を読み込むフラグ
						}
					}
					else{
						if (c == ipd[0] && redFlg == true){
							redFlg = false;
						}
						else{
							redFlg = true;
						}
						rwriteFlg = false;
						ipdCnt = 0;		//\r\nIPD+,検索条件の初期化
					}

					if (rwriteFlg){
						if (recCnt < 0){
							recCnt = 0;
						}
						else{
							recv[recCnt] = c;
							recCnt++;
							if (recCnt >= 256){ break; }
						}
					}
					else{
						if (recCnt > 0){
							for (int i = 0; i < recCnt; i++){
								WiFiData[wifCnt] = recv[i];
								wifCnt++;
								if (wifCnt >= 256){ break; }
							}
							recCnt = 0;
						}
						if (wifCnt >= 256){ break; }

						if (redFlg){
							WiFiData[wifCnt] = c;
							wifCnt++;
							if (wifCnt >= 256){ break; }
						}
					}
					times = millis();
					wait_msec = 1000;	//データが届き始めたら、1sec待ちに変更する
				}
				if (wifCnt >= 256){ break; }
				if (recCnt >= 256){ break; }
			}

			//GETが無いときはそのまま返します。
			if (wifCnt < 6){
				arv[0] = mrb_str_new_cstr(mrb, (const char*)WiFiData);
				arv[1] = mrb_fixnum_value(sesnum);
				return mrb_ary_new_from_values(mrb, 2, arv);
			}

			//"GET /"を探します
			unsigned char *uc;
			int posi = -1;
			for (int i = 0; i < wifCnt - 5; i++){
				if (WiFiData[i] == 'G' && WiFiData[i + 1] == 'E' && WiFiData[i + 2] == 'T' && WiFiData[i + 3] == ' ' && WiFiData[i + 4] == '/'){
					posi = i + 4;
					break;
				}
			}

			if (posi == -1){
				arv[0] = mrb_str_new_cstr(mrb, (const char*)WiFiData);
				arv[1] = mrb_fixnum_value(sesnum);
				return mrb_ary_new_from_values(mrb, 2, arv);
			}

			//スペースを検索します
			for (int i = posi; i < wifCnt; i++){
				if (WiFiData[i] == ' '){
					WiFiData[i] = 0;
					break;
				}
			}

			uc = &WiFiData[posi];
			arv[0] = mrb_str_new_cstr(mrb, (const char*)uc);
			arv[1] = mrb_fixnum_value(sesnum);
			return mrb_ary_new_from_values(mrb, 2, arv);
		}
		else{
			//データ無し
			return mrb_fixnum_value(0);
		}
	}

	//****** AT+CIPSERVERコマンド ******
	if (port < 0){
		RbSerial[WIFI_SERIAL]->println("AT+CIPSERVER=0");
	}
	else{
		RbSerial[WIFI_SERIAL]->print("AT+CIPSERVER=1,");
		RbSerial[WIFI_SERIAL]->println(port);
	}

	//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読むか、指定されたシリアルポートに出力します
	getData(WIFI_WAIT_MSEC);
	return mrb_str_new_cstr(mrb, (const char*)WiFiData);
}

const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define is_base64(c) ( \
    isalnum((unsigned char)c) || \
    ((unsigned char)c == '+') || \
    ((unsigned char)c == '/'))

#define is_newline(c) ( \
    ((unsigned char)c == '\n') || \
    ((unsigned char)c == '\r'))

//**************************************************
// base64エンコードします
//**************************************************
mrb_value base64_encode(mrb_state *mrb, char const* sourFile, char const* descFile)
{
	unsigned char char_array_3[3] = { 0 };
	unsigned char char_array_4[4] = { 0 };
	File frd, fwrt;

	//読込みファイルをオープンします
	if (!(frd = SD.open(sourFile, FILE_READ))){
		return mrb_fixnum_value(2);
	}

	SD.remove(descFile);		//受信するためファイルを事前に消している

	if (!(fwrt = SD.open(descFile, FILE_WRITE))){
		frd.close();
		return mrb_fixnum_value(2);
	}

	//int tLED = 1;
	int i = 0;
	int j = 0;
	long size = frd.size();
	for (long k = 0; k < size; k++){
		char_array_3[i++] = (unsigned char)frd.read();
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			fwrt.write((unsigned char*)&base64_chars[char_array_4[0]], 1);
			fwrt.write((unsigned char*)&base64_chars[char_array_4[1]], 1);
			fwrt.write((unsigned char*)&base64_chars[char_array_4[2]], 1);
			fwrt.write((unsigned char*)&base64_chars[char_array_4[3]], 1);
			i = 0;

			//digitalWrite(RB_LED, tLED);
			//tLED = 1 - tLED;
		}
	}

	if (i) {
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++){
			fwrt.write((unsigned char*)&base64_chars[char_array_4[j]], 1);
		}

		while ((i++ < 3)){
			fwrt.write('=');
		}
	}

	fwrt.flush();
	fwrt.close();
	frd.close();

	return mrb_fixnum_value(0);
}

//**************************************************
// base64デコードします
//**************************************************
mrb_value base64_decode(mrb_state *mrb, char const* sourFile, char const* descFile)
{
	unsigned char char_array_3[3] = { 0 };
	unsigned char char_array_4[4] = { 0 };
	File frd, fwrt;

	//読込みファイルをオープンします
	if (!(frd = SD.open(sourFile, FILE_READ))){
		return mrb_fixnum_value(2);
	}

	SD.remove(descFile);		//受信するためファイルを事前に消している

	if (!(fwrt = SD.open(descFile, FILE_WRITE))){
		frd.close();
		return mrb_fixnum_value(2);
	}

	//int tLED = 1;
	int i = 0;
	int j = 0;
	unsigned char c;
	long size = frd.size();
	for (long k = 0; k < size; k++){
		c = (unsigned char)frd.read();

		if (!is_newline(c)){
			if (c == '=' || !is_base64(c)){
				break;
			}
		}

		if (!is_newline(c)){
			char_array_4[i++] = c;
		}

		if (i == 4) {
			for (i = 0; i < 4; i++){
				char_array_4[i] = strchr(base64_chars, char_array_4[i]) - base64_chars;
			}
			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			fwrt.write((unsigned char*)&char_array_3[0], 1);
			fwrt.write((unsigned char*)&char_array_3[1], 1);
			fwrt.write((unsigned char*)&char_array_3[2], 1);
			i = 0;

			//digitalWrite(RB_LED, tLED);
			//tLED = 1 - tLED;
		}
	}
	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = strchr(base64_chars, char_array_4[j]) - base64_chars;

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++){
			fwrt.write((unsigned char*)&char_array_3[j], 1);
		}
	}


	fwrt.flush();
	fwrt.close();
	frd.close();

	return mrb_fixnum_value(0);
}

//**************************************************
// BASE64変換を行う: WiFi.base64
//  WiFi.base64( SoFile, DeFile[, decode] )
//  Sofile: 入力ファイル名
//  DeFile: 出力ファイル名
//  decode: 0:エンコード, 1:デコード
//          省略時はエンコードします。
//
//　戻り値
//　0: 成功しました
//  1: SDカードがありません
//  2: ファイルのオープンに失敗しました
//  
//**************************************************
mrb_value mrb_wifi_base64(mrb_state *mrb, mrb_value self)
{
mrb_value vsour, vdesc;
char *fsour;
char *fdesc;
int decode = 0;

	int n = mrb_get_args(mrb, "SS|i", &vsour, &vdesc, &decode);

	if (n < 3){
		decode = 0;
	}

	fsour = RSTRING_PTR(vsour);
	fdesc = RSTRING_PTR(vdesc);

	//SDカードが利用可能か確かめます
	if (!sdcard_Init(mrb)){
		return mrb_fixnum_value(1);
	}

	if (decode == 0){
		//エンコードします
		return base64_encode(mrb, (char const*)fsour, (char const*)fdesc);
	}

	//デコードします
	return base64_decode(mrb, (char const*)fsour, (char const*)fdesc);
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
	int ret;
	int cnt = 0;

	while (true){
		//受信バッファを空にします
		while ((len = RbSerial[WIFI_SERIAL]->available()) > 0){
			//RbSerial[0]->print(len);
			for (int i = 0; i < len; i++){
				RbSerial[WIFI_SERIAL]->read();
			}
		}

		//ECHOオフコマンドを送信する
		RbSerial[WIFI_SERIAL]->println("ATE0");

		//OK 0d0a か ERROR 0d0aが来るまで WiFiData[]に読む
		ret = getData(500);
		if (ret == 1){
			//1の時は、WiFiが使用可能
			break;
		}
		else if (ret == 0){
			//タイムアウトした場合は WiFiが使えないとする
			return 0;
		}

		//0,1で無いときは256バイト以上が返ってきている
		cnt++;
		if (cnt >= 3){
			//3回ATE0を試みてダメだったら、あきらめる。
			return 0;
		}
	}

	struct RClass *wifiModule = mrb_define_module(mrb, WIFI_CLASS);

	mrb_define_module_function(mrb, wifiModule, "at", mrb_wifi_at, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "serialOut", mrb_wifi_Sout, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "cwmode", mrb_wifi_Cwmode, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, wifiModule, "setMode", mrb_wifi_Cwmode, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "cwjap", mrb_wifi_Cwjap, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, wifiModule, "connect", mrb_wifi_Cwjap, MRB_ARGS_REQ(2));

	mrb_define_module_function(mrb, wifiModule, "cifsr", mrb_wifi_Cifsr, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, wifiModule, "ipconfig", mrb_wifi_Cifsr, MRB_ARGS_NONE());

	mrb_define_module_function(mrb, wifiModule, "multiConnect", mrb_wifi_multiConnect, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "softAP", mrb_wifi_softAP, MRB_ARGS_REQ(4));
	mrb_define_module_function(mrb, wifiModule, "connectedIP", mrb_wifi_connectedIP, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, wifiModule, "dhcp", mrb_wifi_dhcp, MRB_ARGS_REQ(2));

	mrb_define_module_function(mrb, wifiModule, "httpGetSD", mrb_wifi_getSD, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "httpGet", mrb_wifi_get, MRB_ARGS_REQ(1) | MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "udpOpen", mrb_wifi_udpOpen, MRB_ARGS_REQ(4));

	mrb_define_module_function(mrb, wifiModule, "send", mrb_wifi_send, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "recv", mrb_wifi_recv, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "httpPostSD", mrb_wifi_postSD, MRB_ARGS_REQ(3));
	mrb_define_module_function(mrb, wifiModule, "httpPost", mrb_wifi_post, MRB_ARGS_REQ(3));

	mrb_define_module_function(mrb, wifiModule, "cClose", mrb_wifi_cClose, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, wifiModule, "httpServerSD", mrb_wifi_serverSD, MRB_ARGS_OPT(1));
	mrb_define_module_function(mrb, wifiModule, "httpServer", mrb_wifi_server, MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "version", mrb_wifi_Version, MRB_ARGS_NONE());
	mrb_define_module_function(mrb, wifiModule, "disconnect", mrb_wifi_Disconnect, MRB_ARGS_NONE());

	mrb_define_module_function(mrb, wifiModule, "base64", mrb_wifi_base64, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, wifiModule, "bypass", mrb_wifi_bypass, MRB_ARGS_NONE());

	return 1;
}
