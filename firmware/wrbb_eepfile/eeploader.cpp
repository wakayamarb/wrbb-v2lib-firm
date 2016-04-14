/*
 * EEPROM FIle Loader
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <reboot.h>

#include <eepfile.h>

#include <mruby.h>
#include <mruby/string.h>
#include <mruby/variable.h>
#include <mruby/version.h>

#include <eeploader.h>
#include "../wrbb.h"

#define COMMAND_LENGTH	32

extern char RubyFilename[];
extern uint8_t RubyCode[];


char *WriteData = (char*)RubyCode;
char CommandData[COMMAND_LENGTH];
bool StopFlg = false;		//強制終了フラグ

HardwareSerial *USB_Serial = &Serial;

//#define DEBUG				// Define if you want to debug

#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

//**************************************************
// ライン入力
//**************************************************
void lineinput(char *arry)
{
	int len = 0; 
	int k = 0;
	arry[len] = 0;
	int loopCnt = 0;

	k = USB_Serial->read();
	DEBUG_PRINT("0:USB_Serial->read", k);
	//バッファにH か0x0Dか0x0Dがあるときにはバッファをクリアした後returnさせるため、arry[0]に0x0Dを入れる。
	if(k == 'H' || k == 0x0D || k == 0x0A){
		arry[len] = 0x0D;
	}

	while(k >= 0){
		k = USB_Serial->read();
		delay(4);
		DEBUG_PRINT("1:USB_Serial->read", k);
	}

	if(arry[len] == 0x0D){
		arry[len] = 0;
		return;
	}

	while(true){
		k = 0;
		while(k <= 0 ){
			k = USB_Serial->read();
			delay(8);
		}
		DEBUG_PRINT("2:USB_Serial->read", k);

		if (k == 0x0D || k == 0x0A){	//改行
			break;
		}
		else if (k == 0x08){			//BS
			len--;
			if (len < 0){ len = 0; }
		}
		else{
			arry[len] = k;
			len++;
			if (len >= COMMAND_LENGTH){	break;	}
		}
		USB_Serial->print((char)k);
	}

	arry[len] = 0;
}

//**************************************************
// 指定したmsecの間シリアル入力を待ちます
// 戻り値: 0:入力無し, 1:入力あり
//**************************************************
int waitRcv(int msec){

	unsigned long tm = millis() + msec;
	int sa = 0;

	USB_Serial->print("Waiting ");
	while(tm > millis()){
		if (USB_Serial->available() > 0){
			return 1;
		}
		if(sa != (int)(tm - millis()) / 1000){
			sa = (int)(tm - millis()) / 1000;
			USB_Serial->print(" ");
			USB_Serial->print(sa, 10);
		}
	}
	USB_Serial->println("..Wait Error!");
	return 0;
}

//**************************************************
// 16進文字を10進数に変換する
//**************************************************
int HexText2Int(char t0, char t1)
{
	int ans = 0;
	if(t0 >= '0' && t0 <= '9'){
		ans = (t0 - '0') * 16;
	}
	else if(t0 >= 'A' && t0 <= 'F'){
		ans = (t0 - 'A' + 10) * 16;
	}
	else if(t0 >= 'a' && t0 <= 'f'){
		ans = (t0 - 'a' + 10) * 16;
	}

	if(t1 >= '0' && t1 <= '9'){
		ans += t1 - '0';
	}
	else if(t1 >= 'A' && t1 <= 'F'){
		ans += t1 - 'A' + 10;
	}
	else if(t1 >= 'a' && t1 <= 'f'){
		ans += t1 - 'a' + 10;
	}
	return ans;
}

//**************************************************
// ファイルを保存します
// 60sec待って、データが何も送られてこないときには、
// ファイル保存を終了します
// code
//   W:バイナリ読み込み(サイズは自由、mrbファイルとは限らない) 10  11 ...
//   X:バイナリ読み込み(mrbファイルとみなす) 10  11 ...
//   U:テキスト読みこみ(サイズは自由、mrbファイルとは限らない) '0A' '0B' ...
//   V:テキスト読みこみ(mrbファイルがテキスト化されて送られてくるとみなす) '0A' '0B' ...
//
//   X,Vは読み込み後、実行される
//**************************************************
bool writefile(const char *fname, int size, char code, char *readData)
{
	FILEEEP fpj;
	FILEEEP *fp = &fpj;
	unsigned long tm;
	int binsize = size;
	int b2aFlg = 0;
	bool result = false;

	if(code == 'U' || code == 'V'){
		binsize = size / 2;
		b2aFlg = 1;	//バイナリが2バイトのアスキーコードで来ているフラグ
	}

	if(binsize <= 0){ return result; }

	if(binsize>=RUBY_CODE_SIZE && (code == 'X' || code == 'V')){
		return result;
	}

	USB_Serial->println();

	//シリアルバッファ消去
	while(USB_Serial->available()){ USB_Serial->read();	}

	if(waitRcv(60000) == 0){
		return result;
	}
	USB_Serial->println();

	int cnt = 0;
	int len;
	char c;
	tm = millis() + 2000;

	while(cnt < size){
		len = USB_Serial->available();
		if (len > 0){
			for(int i=0; i<len; i++){
				readData[cnt + i] = (char)USB_Serial->read();
			}
			cnt += len;
			tm = millis() + 2000;
		}
		if(tm < millis()){	break;	}
	}

	if(tm < millis()){
		USB_Serial->println("..Read Error!");
	}

	USB_Serial->print(fname);
	USB_Serial->print("(");
	USB_Serial->print(binsize);

	if(tm < millis()){
		USB_Serial->print(") Saving the reading part");
	}
	else{
		USB_Serial->print(") Saving");
	}

	if(EEP.fopen(fp, fname, EEP_WRITE) == -1){
		USB_Serial->println("..File Open Error!");
		return result;
	}

	result = true;
	for(int i=0; i<binsize; i++){
		//b2aFlgが 0 のときはバイナリ、1 のときはバイナリが2バイトテキストで送られてくる
		if(b2aFlg == 0){
			//****バイナリ
			c = readData[i];
		}
		else{
			//****テキスト
			c = (char)HexText2Int(readData[2*i], readData[2*i + 1]);
		}

		if(EEP.fwrite(fp, c) == -1){
			USB_Serial->println("..Save Error!");
			result = false;
			break;
		}
		if((i % 256) == 0){
			USB_Serial->print(".");
		}
	}
	EEP.fclose(fp);

	USB_Serial->println(".");
	
	return result;
}

//**************************************************
// ファイルを読み出します
// 60sec待って、データが何も送られてこないときには、
// ファイル読み出しを終了します
//**************************************************
void readfile(const char *fname, char code)
{
	FILEEEP fpj;
	FILEEEP *fp = &fpj;
	int k = 0;
	int binsize;

	USB_Serial->println();

	//シリアルバッファ消去
	while(k >= 0){	k = USB_Serial->read();	}

	//60sec間入力を待ちます
	if(waitRcv(60000) == 0){
		return;
	}

	//シリアルバッファ消去
	k = USB_Serial->read();
	while(k >= 0){	k = USB_Serial->read();	}

	USB_Serial->println();

	//ファイルサイズの取得
	binsize = EEP.ffilesize(fname);

	//ファイルサイズの送信
	USB_Serial->println(binsize, 10);

	//60sec間入力を待ちます
	if(waitRcv(60000) == 0){
		return;
	}

	USB_Serial->println();

	//シリアルバッファ消去
	k = USB_Serial->read();
	while(k >= 0){	k = USB_Serial->read();	}

	//ファイルオープン
	if(EEP.fopen(fp, fname, EEP_READ) == -1){
		USB_Serial->println("..Read Error!");
		return;
	}

	for(int i=0; i<binsize; i++){

		if(code == 'G'){
			USB_Serial->write((unsigned char)EEP.fread(fp));
		}
		else{
			int bin = EEP.fread(fp);
			if(bin < 0x10){
				USB_Serial->print("0");
			}
			USB_Serial->print(bin, 16);
		}
	}
	EEP.fclose(fp);
}

//**************************************************
// ファイルローダー
// 戻り値 0:何もしない, 1:強制終了する
//**************************************************
int fileloader(const char* str0, const char* str1)
{
#if BOARD == BOARD_GR
	int led = digitalRead(PIN_LED0) | ( digitalRead(PIN_LED1)<<1) | (digitalRead(PIN_LED2)<<2)| (digitalRead(PIN_LED3)<<3);
#else
	int led = digitalRead(RB_LED);
#endif

	char fname[COMMAND_LENGTH];
	int size = 0;
	char tc[2];
	char *fs[4];

	StopFlg = false;

	tc[0] = CommandData[0];
	tc[1] = CommandData[1];

	while(true){

		//LEDを点灯する
#if BOARD == BOARD_GR
		digitalWrite(PIN_LED0, HIGH);
		digitalWrite(PIN_LED1, HIGH);
		digitalWrite(PIN_LED2, HIGH);
		digitalWrite(PIN_LED3, HIGH);
#else
		digitalWrite(RB_LED, HIGH);
#endif
		//コマンド待ち
		USB_Serial->println();
		USB_Serial->print("WAKAYAMA.RB Board Ver.");
		USB_Serial->print(str0);
		if(str1[0] != 0){
			USB_Serial->print(", mruby ");
			USB_Serial->print(str1);
		}
		USB_Serial->println(" (H [ENTER])");
		USB_Serial->print(">>");
		USB_Serial->flush();

		lineinput((char*)CommandData);

		USB_Serial->println();

		if(CommandData[0] == '.'){
			CommandData[0] = tc[0];
			CommandData[1] = tc[1];

			USB_Serial->print(">");
			USB_Serial->println((char*)CommandData);
		}
		tc[0] = CommandData[0];
		tc[1] = CommandData[1];

		if (CommandData[0] == 'Z'){
			EEP.format();
		}
		else if (CommandData[0] == 'D'){
			if(strlen(CommandData) > 2){

				//ファイル名を取得
				int len = strlen(CommandData);
				for(int i=0; i<len; i++){
					if(CommandData[i] == ' '){
						fs[0] = &CommandData[i+1];
						break;
					}
				}
				strcpy(fname, fs[0]);
				EEP.fdelete((const char*)fname);
			}
		}
		else if (CommandData[0] == 'G' || CommandData[0] == 'F'){
			if(strlen(CommandData) > 2){

				//ファイル名を取得
				int len = strlen(CommandData);
				for(int i=0; i<len; i++){
					if(CommandData[i] == ' '){
						fs[0] = &CommandData[i+1];
						break;
					}
				}
				strcpy(fname, fs[0]);

				readfile(fname, CommandData[0]);
			}
		}
		else if (CommandData[0] == 'R'){
			if(strlen(CommandData) > 2){

				//ファイル名を取得
				int len = strlen(CommandData);
				for(int i=0; i<len; i++){
					if(CommandData[i] == ' '){
						fs[0] = &CommandData[i+1];
						break;
					}
				}
				strcpy(fname, fs[0]);
				strcpy( (char*)RubyFilename, fname );

				len = strlen(RubyFilename);
				if(RubyFilename[len-4] != '.'
					|| RubyFilename[len-3] != 'm'
					|| RubyFilename[len-2] != 'r'
					|| RubyFilename[len-1] != 'b'){
	
					strcat(RubyFilename, ".mrb");				
				}

				//強制終了フラグを立てる
				StopFlg = true;
				break;
			}
		}
		else if(CommandData[0] == 'W' || CommandData[0] == 'U'){
			if(strlen(CommandData) > 3){
				//スペースを0に変えて、ポインタを取得
				int j = 0;
				int len = strlen(CommandData);
				for(int i=0; i<len; i++){
					if(CommandData[i] == ' '){
						CommandData[i] = 0;
						fs[j] = &CommandData[i+1];
						j++;
						if(j>2){	break;	}
					}
				}
				strcpy(fname, fs[0]);
				size = atoi(fs[1]);

				//ヒープメモリの確保
				char *readData = (char*)malloc(size);
				if(readData == NULL){
					USB_Serial->println("..Out of Memory!");
				}
				else{
					//ファイルを保存します
					writefile(fname, size, CommandData[0], readData);
					free( readData );
				}

				for(int i=0; i<j; i++){ *(fs[i] - 1) = ' '; }
			}
		}
		else if(CommandData[0] == 'X' || CommandData[0] == 'V'){
			if(strlen(CommandData) > 3){
				//スペースを0に変えて、ポインタを取得
				int j = 0;
				int len = strlen(CommandData);
				for(int i=0; i<len; i++){
					if(CommandData[i] == ' '){
						CommandData[i] = 0;
						fs[j] = &CommandData[i+1];
						j++;
						if(j>2){	break;	}
					}
				}
				strcpy(fname, fs[0]);
				size = atoi(fs[1]);

				strcpy( (char*)RubyFilename, fname );

				len = strlen(RubyFilename);
				if(RubyFilename[len-4] != '.'
					|| RubyFilename[len-3] != 'm'
					|| RubyFilename[len-2] != 'r'
					|| RubyFilename[len-1] != 'b'){
	
					strcat(RubyFilename, ".mrb");				
				}

				//ヒープメモリの確保
				char *readData = (char*)malloc(size);
				if(readData == NULL){
					USB_Serial->println("..Out of Memory!");
				}
				else{
					//ファイルを保存します。保存に成功すれば、強制終了フラグが立ちます
					StopFlg = writefile(RubyFilename, size, CommandData[0], readData);
					free( readData );
				}

				for(int i=0; i<j; i++){ *(fs[i] - 1) = ' '; }
				break;
			}
		}
		else if(CommandData[0] == 'E'){
			//ファームウェア書き込み待ちにする
			system_reboot( REBOOT_USERAPP );	//リセット後にユーザアプリを起動する
			//system_reboot( REBOOT_FIRMWARE );	//リセット後にファームウェアを起動する
		}
		else if (CommandData[0] == 'L'){
			USB_Serial->println();
			for(int i=0; i<64; i++){
					
				size = EEP.fdir( i, fname );
					
				if(fname[0] != 0){
					USB_Serial->print(" ");
					USB_Serial->print(fname);
					USB_Serial->print(" ");
					USB_Serial->print(size);
					USB_Serial->println(" byte");
				}
			}
		}
		else if(CommandData[0] == 'A'){
			EEP.viewFat();
		}
		else if(CommandData[0] == 'S'){
			if(strlen(CommandData) > 2){

				int len = strlen(CommandData);
				for(int i=0; i<len; i++){
					if(CommandData[i] == ' '){
						fs[0] = &CommandData[i+1];
						break;
					}
				}
				EEP.viewSector( atoi(fs[0]) );
			}
		}
		else if(CommandData[0] == 'Q'){
			break;
		}
		else if (CommandData[0] == 'C'){
			USB_Serial->println();
			USB_Serial->println(LICENSE_MRUBY);
			USB_Serial->println(LICENSE_MRUBYURL);
			USB_Serial->println(LICENSE_WRBB);
			USB_Serial->println(LICENSE_WRBBURL);
		}
		else{
			USB_Serial->println();
			USB_Serial->println("EEPROM FileWriter Ver. 1.74.v2");
			USB_Serial->println(" Command List");
			USB_Serial->println(" L:List Filename..........>L [ENTER]");
			USB_Serial->println(" W:Write File.............>W Filename Size [ENTER]");
			USB_Serial->println(" G:Get File...............>G Filename [ENTER]");
			USB_Serial->println(" F:Get File B2A...........>F Filename [ENTER]");
			USB_Serial->println(" D:Delete File............>D Filename [ENTER]");
			USB_Serial->println(" Z:Delete All Files.......>Z [ENTER]");
			USB_Serial->println(" A:List FAT...............>A [ENTER]");
			USB_Serial->println(" R:Run File...............>R Filename [ENTER]");
			USB_Serial->println(" X:Execute File...........>X Filename Size [ENTER]");
			USB_Serial->println(" S:List Sector............>S Number [ENTER]");
			USB_Serial->println(" .:Repeat.................>. [ENTER]");
			USB_Serial->println(" Q:Quit...................>Q [ENTER]");
			USB_Serial->println(" E:System Reset...........>E [ENTER]");
			USB_Serial->println(" U:Write File B2A.........>U Filename Size [ENTER]");
			//USB_Serial->println(" V:Execute File B2A.......>V Filename Size [ENTER]");
			USB_Serial->println(" C:License................>C [ENTER]");
		}
	}

#if BOARD == BOARD_GR
	digitalWrite(PIN_LED0, led & 1);
	digitalWrite(PIN_LED1, (led >> 1) & 1);
	digitalWrite(PIN_LED2, (led >> 2) & 1);
	digitalWrite(PIN_LED3, (led >> 3) & 1);
#else
	digitalWrite(RB_LED, led);
#endif

	if(StopFlg == true){
		return 1;
	}

	return 0;
}
