/*
 * WRBB Main
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>

#include <eepfile.h>
#include <eeploader.h>
#include <iodefine_gcc63n.h>

#include <mruby/version.h>
#include <mruby.h>

#include <sExec.h>
#include "wrbb.h"

char RubyStartFileName[RUBY_FILENAME_SIZE];	//xmlに指定された最初に起動するmrubyファイル名
char RubyFilename[RUBY_FILENAME_SIZE];
char ExeFilename[RUBY_FILENAME_SIZE];		//現在実行されているファイルのパス名

extern volatile char ProgVer[];

//**********************************
//初期化を行います
//**********************************
void init_vm( void )
{
char dat[4];
int en;
int i;

	//EEPファイル関連の初期化
	EEP.begin();

	//スタートファイル名を読み込みます
	RubyStartFileName[0] = 0;

	FILEEEP fpj;
	FILEEEP *fp = &fpj;

	//スタートファイル名を読み込みます
	if(EEP.fopen( fp, XML_FILENAME, EEP_READ ) == -1){
		strcpy( RubyStartFileName, RUBY_FILENAME );
	}
	else{
		//file と ack が出るまでひたすら読み込みます
		int pos = 0;
		while( !EEP.fEof(fp) ){
			EEP.fseek(fp, pos, EEP_SEEKTOP);

			dat[0] = 0;
			dat[1] = 0;
			dat[2] = 0;
			dat[3] = 0;
			for( i=0; i< 4; i++ ){
				en = EEP.fread(fp);
				if( en<0 ){ break; }
				dat[i] = (char)en;
			}

			if( RubyStartFileName[0]==0 && dat[0]=='f' && dat[1]=='i' && dat[2]=='l' && dat[3]=='e'  ){

				//見つかったので " or ' まで読み飛ばす
				while( !EEP.fEof(fp) ){
					en = EEP.fread(fp);
					if( en<0 ){ break; }
					if( (char)en==0x22 || (char)en==0x27 ){

						//見つかったので " or ' まで取り込みます
						for( i=0; i<EEPFILENAME_SIZE; i++ ){
							en = EEP.fread(fp);
							if( en<0 ){
								RubyStartFileName[0] = 0;
								break;
							}

							if( (char)en==0x22 || (char)en==0x27 ){
								RubyStartFileName[i] = 0;
								break;
							}
							RubyStartFileName[i] = (char)en;
						}
						break;
					}
				}
			}
			pos++;
		}
		EEP.fclose(fp);
	}

	if(RubyStartFileName[0] == 0){
		strcpy( RubyStartFileName, RUBY_FILENAME );
	}

	//RubyFilenameにスタートファイル名をコピーします
	strcpy( RubyFilename, RubyStartFileName );
}

//**************************************************
// セットアップします
//**************************************************
void setup()
{
    pinMode(RB_LED, OUTPUT);

#if BOARD == BOARD_GR
    pinMode(PIN_LED0, OUTPUT);
    pinMode(PIN_LED1, OUTPUT);
    pinMode(PIN_LED2, OUTPUT);
    pinMode(PIN_LED3, OUTPUT);
#endif

	//ピンモードを入力に初期化します
	pinModeInit();
	
	//シリアル通信の初期化
	Serial.begin(115200);		//仮想USBシリアル
    //Serial.setDefault();
	//sci_convert_crlf_ex(Serial.get_handle(), CRLF_NONE, CRLF_NONE);	//バイナリを通せるようにする

	//vmの初期化
	init_vm();

	//Port 3-5がHIGHだったら、EEPROMファイルローダーに飛ぶ
	if( FILE_LOAD == 1 ){
		fileloader((const char*)ProgVer,MRUBY_VERSION);
	}
}

//**************************************************
// 無限ループです
//**************************************************
void loop()
{
	if( RubyRun()==false ){

		DEBUG_PRINT("RubyRun", "FALSE");
		fileloader((const char*)ProgVer,MRUBY_VERSION);
	}
}