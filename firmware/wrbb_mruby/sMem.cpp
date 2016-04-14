/*
 * ファイル関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <eepfile.h>

#include <mruby.h>
#include <mruby/string.h>

#include "../wrbb.h"

FILEEEP Fpj0;
FILEEEP *Fp0 = &Fpj0;			//コマンド用
FILEEEP Fpj1;
FILEEEP *Fp1 = &Fpj1;			//コマンド用

//**************************************************
// openしたファイルから1バイト読み込みます: MemFile.read
//	MemFile.read( number )
//	number: ファイル番号 0 または 1
// 戻り値
//	0x00～0xFFが返る。ファイルの最後だったら-1が返る。
//**************************************************
mrb_value mrb_mem_read(mrb_state *mrb, mrb_value self)
{
int	num;

	mrb_get_args(mrb, "i", &num);

	int dat = -1;
	if( num==0 ){
		dat = EEP.fread(Fp0);
	}
	else if( num==1 ){
		dat = EEP.fread(Fp1);
	}
	
	return mrb_fixnum_value( dat );
}

//**************************************************
// openしたファイルバイナリデータを書き込む: MemFile.write
//	MemFile.write( number, buf, len )
//	number: ファイル番号 0 または 1
//	buf: 書き込むデータ
//	len: 書き込むデータサイズ
// 戻り値
//	実際に書いたバイト数
//**************************************************
mrb_value mrb_mem_write(mrb_state *mrb, mrb_value self)
{
int	num, len;
mrb_value value;
char	*str;

	mrb_get_args(mrb, "iSi", &num, &value, &len);

	str = RSTRING_PTR(value);

	int ret = 0;
	if( num==0 ){
		EEP.fwrite(Fp0, str, &len);
		ret = len;
	}
	else if( num==1 ){
		EEP.fwrite(Fp1, str, &len);
		ret = len;
	}

	return mrb_fixnum_value( ret );
}

//**************************************************
// ファイルをオープンします: MemFile.open
//	MemFile.open( number, filename[, mode] )
//	number: ファイル番号 0 または 1
//	filename: ファイル名(8.3形式)
//	mode: 0:Read, 1:Append, 2:New Create
// 戻り値
//	成功: 番号, 失敗: -1
//**************************************************
mrb_value mrb_mem_open(mrb_state *mrb, mrb_value self)
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

	int ret = -1;
	if( num==0 ){
		if( mode==1 ){
			if(EEP.fopen(Fp0, str, EEP_APPEND) != -1){
				ret = num;
			}
		}
		else if( mode==2 ){
			if(EEP.fopen(Fp0, str, EEP_WRITE) != -1){
				ret = num;
			}
		}
		else{
			if(EEP.fopen(Fp0, str, EEP_READ) != -1){
				ret = num;
			}
		}
	}
	else if( num==1 ){
		if( mode==1 ){
			if(EEP.fopen(Fp1, str, EEP_APPEND) != -1){
				ret = num;
			}
		}
		else if( mode==2 ){
			if(EEP.fopen(Fp1, str, EEP_WRITE) != -1){
				ret = num;
			}
		}
		else{
			if(EEP.fopen(Fp1, str, EEP_READ) != -1){
				ret = num;
			}
		}
	}

	return mrb_fixnum_value( ret );
}


//**************************************************
// ファイルをクローズします: MemFile.close( number )
//	MemFile.close( number )
//	number: ファイル番号 0 または 1
//**************************************************
mrb_value mrb_mem_close(mrb_state *mrb, mrb_value self)
{
int	num;

	mrb_get_args(mrb, "i", &num);

	if( num==0 ){
		EEP.fclose(Fp0);
	}
	else if( num==1 ){
		EEP.fclose(Fp1);
	}

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// openしたファイルの読み出し位置を移動する: MemFile.seek
//	MemFile.seek( number, byte )
//	number: ファイル番号 0 または 1
//	byte: seekするバイト数(-1)でファイルの最後に移動する
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_mem_seek(mrb_state *mrb, mrb_value self)
{
int	num;
int size;
int ret = 0;

	mrb_get_args(mrb, "ii", &num, &size);

	if( num==0 ){
		if( size==-1 ){
			if( EEP.fseek(Fp0, 0, EEP_SEEKEND) ){
				ret = 1;
			}
		}
		else{
			if( EEP.fseek(Fp0, size, EEP_SEEKTOP) ){
				ret = 1;
			}
		}
	}
	else if( num==1 ){
		if( size==-1 ){
			if( EEP.fseek(Fp1, 0, EEP_SEEKEND) ){
				ret = 1;
			}
		}
		else{
			if( EEP.fseek(Fp1, size, EEP_SEEKTOP) ){
				ret = 1;
			}
		}
	}
	return mrb_fixnum_value( ret );
}

//**************************************************
// ファイルをコピーします: MemFile.cp
//  MemFile.cp( srcFilename, dstFilename[, mode] )
//  srcFilename: コピー元ファイル名
//  dstFilename: コピー先ファイル名
//  mode: 0上書きしない, 1:上書きする
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_mem_cp(mrb_state *mrb, mrb_value self)
{
mrb_value src, dst;
int mode;
int ret = 1;

	int n = mrb_get_args(mrb, "SS|i", &src, &dst, &mode);
	
	//DEBUG_PRINT("MemFile.copy", n);

	if( n<3 ){
		mode = 0;
	}

	if(mode == 0){
		if(!EEP.fexist(RSTRING_PTR(dst))){
			mode = 1;
		}
	}

	//DEBUG_PRINT("mode", mode);

	if(mode == 1){
		if(EEP.fcopy(RSTRING_PTR(src), RSTRING_PTR(dst)) == -1){
			ret = 0;
		}
	}
	return mrb_fixnum_value( ret );
}

//**************************************************
// ファイルを削除します: MemFile.rm
//  MemFile.rm( Filename )
//  Filename: 削除するファイル名
// 戻り値
//	成功: 1, 失敗: 0
//**************************************************
mrb_value mrb_mem_rm(mrb_state *mrb, mrb_value self)
{
mrb_value src;
int ret = 1;

	mrb_get_args(mrb, "S", &src);
	
	if(EEP.fdelete(RSTRING_PTR(src)) == -1){
		ret = 0;
	}
	return mrb_fixnum_value( ret );
}

//**************************************************
// ライブラリを定義します
//**************************************************
void mem_Init(mrb_state *mrb)
{
	struct RClass *memdModule = mrb_define_module(mrb, "MemFile");

	mrb_define_module_function(mrb, memdModule, "read", mrb_mem_read, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, memdModule, "seek", mrb_mem_seek, MRB_ARGS_REQ(2));

	mrb_define_module_function(mrb, memdModule, "write", mrb_mem_write, MRB_ARGS_REQ(3));

	mrb_define_module_function(mrb, memdModule, "open", mrb_mem_open, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, memdModule, "close", mrb_mem_close, MRB_ARGS_REQ(1));

	mrb_define_module_function(mrb, memdModule, "cp", mrb_mem_cp, MRB_ARGS_REQ(2) | MRB_ARGS_OPT(1));

	mrb_define_module_function(mrb, memdModule, "rm", mrb_mem_rm, MRB_ARGS_REQ(1));
}
