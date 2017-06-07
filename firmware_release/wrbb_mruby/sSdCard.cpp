/*
 * SDカード関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <SD.h>
#include <RTC.h>
#include <eepfile.h>

#include "mruby.h"
#include "mruby/string.h"

#include "../wrbb.h"
#include "sSdCard.h"

File Fp[2];
bool SdBeginFlag = false;	//SD.begin()で1が返ってきたら true となる
bool SdClassFlag = false;

//**************************************************
//ファイルが存在するかどうか調べる: SD.exists
//	SD.exists( filename )
//  filename: 調べるファイル名
//
// 戻り値
//	存在する: 1, 存在しない: 0
//**************************************************
mrb_value mrb_sdcard_exists(mrb_state *mrb, mrb_value self)
{
mrb_value value;
char	*str;
int ret = 0;

	mrb_get_args(mrb, "S", &value);

	str = RSTRING_PTR(value);

	if( SD.exists( str )==true){
		ret = 1;
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
// ディレクトリを作成する: SD.mkdir
//	SD.mkdir( dirname )
//  dirname: 作成するディレクトリ名
//
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_sdcard_mkdir(mrb_state *mrb, mrb_value self)
{
mrb_value value;
char	*str;
int ret = 0;

	mrb_get_args(mrb, "S", &value);

	str = RSTRING_PTR(value);

	if( SD.mkdir( str )==true){
		ret = 1;
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
// ファイルを削除する: SD.remove
//	SD.remove( filename )
//  filename: 削除するファイル名
//
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_sdcard_remove(mrb_state *mrb, mrb_value self)
{
mrb_value value;
char	*str;
int ret = 0;

	mrb_get_args(mrb, "S", &value);

	str = RSTRING_PTR(value);

	if( SD.remove( str )==true){
		ret = 1;
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
// ファイルをコピーする: SD.copy
//	SD.copy( srcfilename, distfilename )
//  srcfilename: コピー元ファイル名
//  distfilename: コピー先ファイル名
//
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_sdcard_copy(mrb_state *mrb, mrb_value self)
{
mrb_value value1, value2;
char	*str1;
char	*str2;
int ret = 0;
int v;
unsigned char chr[1];

	mrb_get_args(mrb, "SS", &value1, &value2);

	str1 = RSTRING_PTR(value1);
	str2 = RSTRING_PTR(value2);

	if( SD.exists( str2 )==true){
		SD.remove(str2);
	}

	//既存のファイルをコピーする
	File s = SD.open(__null);
	File d = SD.open(__null);

	if( s = SD.open( str1, FILE_READ )){
		if( d = SD.open( str2, FILE_WRITE )){
			v = s.read();
			while(v >=0 ){
				chr[0] = (unsigned char)v;
				d.write(chr, 1);
				v = s.read();
			}
			ret = 1;
			d.flush();
			d.close();
		}
		s.close();
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
// ディレクトリを削除する: SD.rmdir
//	SD.rmdir( dirname )
//  dirname: 削除するディレクトリ名
//
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_sdcard_rmdir(mrb_state *mrb, mrb_value self)
{
mrb_value value;
char	*str;
int ret = 0;

	mrb_get_args(mrb, "S", &value);

	str = RSTRING_PTR(value);

	if( SD.rmdir( str )==true){
		ret = 1;
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
// ファイルをオープンします: SD.open
//	SD.open( number, filename[, mode] )
//	number: ファイル番号 0 または 1
//	filename: ファイル名(8.3形式)
//	mode: 0:Read, 1:Append, 2:New Create
// 戻り値
//	成功: 番号, 失敗: -1
//**************************************************
mrb_value mrb_sdcard_open(mrb_state *mrb, mrb_value self)
{
int	num;
int mode;
mrb_value value;
char	*str;

	int n = mrb_get_args(mrb, "iS|i", &num, &value, &mode);

	str = RSTRING_PTR(value);

	if( n<3 ){
		mode = 0;
	}

	if( mode==2 ){
		//新規書き込み
		if( SD.exists( str )==true){
			SD.remove( str );
		}
	}

	int ret = -1;
	if(num < 0 || num > 1){ return mrb_fixnum_value(ret); }

	if( mode==1 || mode==2 ){
		if( Fp[num] = SD.open( str, FILE_WRITE )){
			Fp[num].seek(Fp[num].size());
			ret = num;
		}
	}
	else{
		if( Fp[num] = SD.open( str, FILE_READ )){
			ret = num;
		}
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
// openしたファイルをクローズします: SD.close
//	SD.close( number )
//	number: ファイル番号 0 または 1
//**************************************************
mrb_value mrb_sdcard_close(mrb_state *mrb, mrb_value self)
{
int	num;

	mrb_get_args(mrb, "i", &num);

	if(num >= 0 && num < 2){
		Fp[num].close();
	}

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// openしたファイルから1バイト読み込みます: SD.read
//	SD.read( number )
//	number: ファイル番号 0 または 1
// 戻り値
//	0x00～0xFFが返る。ファイルの最後だったら-1が返る。
//**************************************************
mrb_value mrb_sdcard_read(mrb_state *mrb, mrb_value self)
{
int	num;

	mrb_get_args(mrb, "i", &num);

	int dat = -1;
	if(num < 0 || num > 1){ return mrb_fixnum_value(dat); }

	dat = Fp[num].read();
	
	return mrb_fixnum_value( dat );
}

//**************************************************
// openしたファイルの読み出し位置を移動する: SD.seek
//	SD.seek( number, byte )
//	number: ファイル番号 0 または 1
//	byte: seekするバイト数(-1)でファイルの最後に移動する
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_sdcard_seek(mrb_state *mrb, mrb_value self)
{
int	num;
int size;
int ret = 0;

	mrb_get_args(mrb, "ii", &num, &size);

	if(num < 0 || num > 1){ return mrb_fixnum_value(ret); }

	if( size==-1 ){
		if( Fp[num].seek(Fp[num].size()) ){
			ret = 1;
		}
	}
	else{
		if( Fp[num].seek(size) ){
			ret = 1;
		}
	}
	return mrb_fixnum_value( ret );
}

//**************************************************
// openしたファイルにバイナリデータを書き込む: SD.write
//	SD.write( number, buf, len )
//	number: ファイル番号 0 または 1
//	buf: 書き込むデータ
//	len: 書き込むデータサイズ
// 戻り値
//	実際に書いたバイト数
//**************************************************
mrb_value mrb_sdcard_write(mrb_state *mrb, mrb_value self)
{
int	num, len;
mrb_value value;
char	*str;

	mrb_get_args(mrb, "iSi", &num, &value, &len);

	str = RSTRING_PTR(value);

	int ret = 0;
	if(num < 0 || num > 1){ return mrb_fixnum_value(ret); }

	ret = Fp[num].write( (unsigned char*)str, len );

	return mrb_fixnum_value( ret );
}

//**************************************************
// openしたファイルの書き込みをフラッシュします: SD.flush
//	SD.flush( number )
//	number: ファイル番号 0 または 1
//**************************************************
mrb_value mrb_sdcard_flush(mrb_state *mrb, mrb_value self)
{
int	num;

	mrb_get_args(mrb, "i", &num);

	if(num >= 0 && num < 2){
		Fp[num].flush();
	}

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// openしたファイルのサイズを取得します: SD.size
//	SD.size( number )
//	number: ファイル番号 0 または 1
//
// 戻り値
//	ファイルサイズ
//**************************************************
mrb_value mrb_sdcard_size(mrb_state *mrb, mrb_value self)
{
int	num;
mrb_int ret = 0;

	mrb_get_args(mrb, "i", &num);

	if(num < 0 || num > 1){ return mrb_fixnum_value(ret); }

	ret = Fp[num].size();

	return mrb_fixnum_value( ret );
}

//**************************************************
// openしたファイルのseek位置を取得します: SD.position
//	SD.position( number )
//	number: ファイル番号 0 または 1
//
// 戻り値
//	シーク位置
//**************************************************
mrb_value mrb_sdcard_position(mrb_state *mrb, mrb_value self)
{
int	num;
mrb_int ret = 0;

	mrb_get_args(mrb, "i", &num);

	if(num < 0 || num > 1){ return mrb_fixnum_value(ret); }

	ret = Fp[num].position();

	return mrb_fixnum_value( ret );
}

//**************************************************
// ファイルをフラッシュメモリにコピーします: SD.cpmem
//  SD.cpmem( SDFilename, MemFilename[, mode] )
//  SDFilename: SDカードのファイル名
//  MemFilename: フラッシュメモリのコピー先ファイル名
//  mode: 0上書きしない, 1:上書きする
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_sdcard_cpmem(mrb_state *mrb, mrb_value self)
{
mrb_value src, dst;
int mode;
int ret = 1;

	int n = mrb_get_args(mrb, "SS|i", &src, &dst, &mode);
	
	if( n<3 ){
		mode = 0;
	}

	if(mode == 0){
		if(!EEP.fexist(RSTRING_PTR(dst))){
			mode = 1;
		}
	}

	if(mode == 1){
		if(SD2EEPROM(RSTRING_PTR(src), RSTRING_PTR(dst)) == 0){
			ret = 0;
		}
	}
	return mrb_fixnum_value( ret );
}

//**************************************************
// ライブラリを定義します
//**************************************************
int sdcard_Init(mrb_state *mrb)
{
	//SDカードライブラリを初期化します
	if(SD_init(NULL) == 0){
		return 0;
	}

	//SDクラスが既に設定されているか
	if(SdClassFlag == true){
		return 1;
	}
	
	//日付と時刻を返す関数を登録
	SdFile::dateTimeCallback( &SD_DateTime );

	struct RClass *sdcardModule = mrb_define_module(mrb, SD_CLASS);

	mrb_define_module_function(mrb, sdcardModule, "exists", mrb_sdcard_exists, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "mkdir", mrb_sdcard_mkdir, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "remove", mrb_sdcard_remove, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "rename", mrb_sdcard_copy, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, sdcardModule, "rmdir", mrb_sdcard_rmdir, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "open", mrb_sdcard_open, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, sdcardModule, "close", mrb_sdcard_close, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "read", mrb_sdcard_read, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "seek", mrb_sdcard_seek, MRB_ARGS_REQ(2));
	mrb_define_module_function(mrb, sdcardModule, "write", mrb_sdcard_write, MRB_ARGS_REQ(3));
	mrb_define_module_function(mrb, sdcardModule, "flush", mrb_sdcard_flush, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "size", mrb_sdcard_size, MRB_ARGS_REQ(1));
	mrb_define_module_function(mrb, sdcardModule, "position", mrb_sdcard_position, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, sdcardModule, "cpmem", mrb_sdcard_cpmem, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(1));

	//SDクラスのセットフラグをtrueにする
	SdClassFlag = true;

	return 1;
}

//**************************************************
// 日付と時刻を返すコールバック関数
//**************************************************
void SD_DateTime(uint16_t *date, uint16_t *time)
{
uint16_t year = 2000;
uint8_t month = 1, day = 1, hour = 0, minute = 0, second = 0;
RTC_TIMETYPE timertc;

	if(rtc_get_time(&timertc)){
		//year = timertc.year + 2000;
		year = timertc.year;
		month = timertc.mon;
		day = timertc.day;
		hour = timertc.hour;
		minute = timertc.min;
		second = timertc.second;
	}

	// FAT_DATEマクロでフィールドを埋めて日付を返す
	*date = FAT_DATE(year, month, day);

	// FAT_TIMEマクロでフィールドを埋めて時間を返す
	*time = FAT_TIME(hour, minute, second);
}

//**************************************************
// SDカードライブラリを初期化します
// filenameが指定されていれば、そのファイルが存在すれば成功を返します
// 失敗 0, 成功 1
//**************************************************
int SD_init(char *filename)
{
	if(!SdBeginFlag && !SD.begin()){
		return 0;
	}
	SdBeginFlag = true;

	//Serial.println("SD find");

	if(filename == NULL){
		return 1;
	}
	
	//SDカードのmrbファイルを探します
	if(SD.exists(filename) == false){
		return 0;
	}	
	return 1;
}

//**************************************************
// SDカードのファイルをフラッシュメモリにコピーします
// 上書きです
// 失敗 0, 成功 1
//**************************************************
int SD2EEPROM(const char *sdfile, const char *eepfile)
{
File fsrc = SD.open(__null);
FILEEEP fdstj;
FILEEEP *fdst = &fdstj;
int dsize;

	//オープンします
	if( !(fsrc = SD.open(sdfile, FILE_READ))){
		return 0;
	}

	//ファイルサイズを取得します
	int fsize = fsrc.size();

	//ヒープメモリの確保
	char *readData = (char*)malloc(fsize);
	if(readData == NULL){
		Serial.println("..Out of Memory!");
		fsrc.close();
		return 0;
	}
	
	//ファイルがあるかもしれないので、とりあえず削除しておきます。なかったら-1が返ってくるだけ
	EEP.fdelete(eepfile);

	//EEPをオープンします
	if(EEP.fopen(fdst, eepfile, EEP_WRITE) == -1){
		fsrc.close();
		free( readData );
		return 0;
	}
	
	//読み込みます
	dsize = fsrc.read(readData, fsize);

	//書き込みます
	EEP.fwrite(fdst, readData, &dsize);

	//ファイルを閉じます
	EEP.fclose(fdst);
	fsrc.close();

	//メモリを開放します
	free( readData );

	return 1;
}
