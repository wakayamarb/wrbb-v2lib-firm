/*
 * EEPROMをファイルのように使うクラス
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
//***********************************************************
// ファイル名
//  先頭セクタに入っている0x00までで31バイト以内
//
// ファイルサイズ
//  先頭セクタのファイル名に続く2バイトに入っている
//
// データ
//  先頭セクタのファイル名に続く2バイト以降に入っている
//
// 先頭セクタ
//  xxxxxxxxxxxxxxxxxxx00 | 0000 | zzzzzzzzzzzzz...
//  ファイル名　　　00終端 サイズ　生データ
//
// Address 0x0000～0x00FFまでは、EEPROMのPush Popに使われる
//***********************************************************
#include <Arduino.h>
#include <EEPROM.h>
#include "EEPROM/utility/r_flash_api_rx600.h"

#include "eepfile.h"
//#define    DEBUG                1        // Define if you want to debug

#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

#define EEPSIZE			0x8000
#define EEPSECTOR_SIZE	512
#define EEPSECTORS		(EEPSIZE / EEPSECTOR_SIZE)

#define EEPSTASECT		1
#define EEPENDSECT		(EEPSECTORS - 1)

#define EEPFAT_START	0x100	//0x100～0x13FまでをFAT保存領域として使っている

#define EEP_EMPTY	0		//未使用
#define EEP_TOP		1		//先頭
#define EEP_USED	2		//使用中

//宣言
EEPFILE	EEP;

//Sect[]→ 0～6ビット(00～7F)が次のセクタを示す。次のセクタが自分自身を指しているときは最終セクタ。
//　　　　　　　　　9,10ビットは、0:未使用、1:先頭、2:使用中 をあらわす。
//					11,12ビットは、0:オープンしていない、1:READオープン、2:WRITE||APPENDオープン をあらわす。
static unsigned short Sect[EEPSECTORS];		//512バイトを1セクタとして管理する。saveFat()のタイミングでEEPROMに保存される。

//******************************************************
// FATセクターを表示します
//******************************************************
void EEPFILE::viewFat(void)
{
	char az[50];

	Serial.println();
	Serial.println("FAT Sector");
	for(long i=0; i<EEPSECTORS; i++){
		sprintf(az, "%02X-%04X ",(int)i ,(int)Sect[i] );
		Serial.print(az);
		if((i % 10) == 9){
			Serial.println();
		}
	}
	Serial.println();
}

//******************************************************
//　セクタデータを表示します
//******************************************************
void EEPFILE::viewSector(int sect)
{
	char az[50];
	Serial.println();
	Serial.print("Sector ");
	Serial.println(sect);

	int add = sect * EEPSECTOR_SIZE;

	Serial.println("ADDR 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
	for(int i=0; i<EEPSECTOR_SIZE/16; i++){

		sprintf(az, "%04X", add + 16 * i);
		Serial.print(az);

		for(int j=0; j<16; j++){
			sprintf(az, " %02X", EEPROM.read(add + 16 * i + j));
			Serial.print(az);
		}
		Serial.println();
	}
}

//******************************************************
// EEPROM FILEシステムを初期化します
//
// clearが1のときは、ファイルシステムを初期化します
//******************************************************
void EEPFILE::begin(int clear)
{
	if (clear == 0 && isReady() == 1){
		// EEPROMからFATを読み込みます
		for (int i=0; i<EEPSECTORS; i++){
			Sect[i] = EEPROM.read( EEPFAT_START + i*2 ) + (EEPROM.read( EEPFAT_START + i*2 + 1 )<<8);
		}
	}
	else{
		// ファイルシステムを初期化します

		for (int i=0; i<EEPSECTORS; i++){
			Sect[i] = (EEP_USED<<8);		//一度、全て使用中にする
		}		

		for (int i=EEPSTASECT; i<=EEPENDSECT; i++){
			Sect[i] = (EEP_EMPTY<<8);		//FAT使用セクタのみ空き状態にする
		}
		saveFat();
	}
}

//******************************************************
// ファイルをオープンします
// mod: 0:read, 1:write新規, 2:Write追記
// エラーの時は、-1を返す
//******************************************************
int EEPFILE::fopen(FILEEEP *file, const char *filename, char mode)
{
	int sect = 0;
	int len = strlen(filename);
	unsigned long add = 0;

	//既にオープンしていないどうかのチェック
	sect = scanFilename(filename);
	if (sect != -1){
		if((Sect[sect] & ((EEP_READ | EEP_WRITE) << 10)) != 0){
			//既にオープンしている
			return -1;
		}
	}

	if (mode == EEP_READ){

		DEBUG_PRINT("mode", (int)mode);
		DEBUG_PRINT("sect", sect);

		//sect = scanFilename(filename);
		if (sect == -1){
			return -1;
		}

		Sect[sect] |= EEP_READ << 10;			//ファイル使用中フラグ

		add = sect * EEPSECTOR_SIZE;
		file->filesize = (EEPROM.read( add + len + 1 ) & 0xFF) + ((EEPROM.read( add + len + 2 ) & 0xFF) << 8);
		file->stasector = sect;
		file->offsetaddress = len + 3;
		file->seek = 0;
		return 0;
	}
	else if (mode == EEP_WRITE || sect == -1){

		DEBUG_PRINT("mode", (int)mode);
		DEBUG_PRINT("sect", sect);

		//ファイルがあるかもしれないので、とりあえず削除しておきます。なかったら-1が返ってくるだけ
		fdelete(filename);

		//空いているセクタを探す
		sect = scanEmptySector((int)millis() & 0x3F);	//(int)millis()&0x3Fは、乱数の要素を含ませているセクタの平滑化利用(ウェアレベリングを考慮)
		if (sect == -1){
			//ファイルがいっぱいだった
			return -1;
		}

		// ファイルポインタの書き込みを行う
		setFile( file, filename, sect, EEP_WRITE );
		return 0;
	}
	else if (mode == EEP_APPEND){

		DEBUG_PRINT("mode", (int)mode);
		DEBUG_PRINT("sect", sect);

		Sect[sect] |= EEP_WRITE << 10;			//ファイル使用中フラグ
		
		add = sect * EEPSECTOR_SIZE;
		file->filesize = (EEPROM.read( add + len + 1 ) & 0xFF) + ((EEPROM.read( add + len + 2 ) & 0xFF) << 8);
		file->stasector = sect;
		file->offsetaddress = len + 3;
		file->seek = file->filesize;
		return 0;
	}
	return -1;
}

//******************************************************
// ファイルのシーク
// orignを基準としたオフセット値に移動する
// ファイルの先頭からの位置を返す
//******************************************************
int EEPFILE::fseek(FILEEEP *file, int offset, int origin)
{
	long sk = 0;
	if (origin == EEP_SEEKTOP){
		sk = 0 + offset;
	}
	else if (origin == EEP_SEEKCUR){
		sk = file->seek + offset;
	}
	else if (origin == EEP_SEEKEND){
		sk = file->filesize + offset;
	}

	if (sk < 0){
		sk = 0;
	}
	else if (sk > file->filesize){
		sk = file->filesize;
	}

	file->seek = sk;

	return file->seek;
}

//******************************************************
// ファイルをコピーします。必ず上書きします。
// エラーの時は、-1を返す
//******************************************************
int EEPFILE::fcopy(const char *srcfilename, const char *dstfilename)
{
FILEEEP fsrcj;
FILEEEP *fsrc = &fsrcj;
FILEEEP fdstj;
FILEEEP *fdst = &fdstj;

	if(fopen(fsrc, srcfilename, EEP_READ) == -1){
		return -1;
	}

	//ファイルがあるかもしれないので、とりあえず削除しておきます。なかったら-1が返ってくるだけ
	fdelete(dstfilename);

	if(fopen(fdst, dstfilename, EEP_WRITE) == -1){
		return -1;
	}

	int dat = fread(fsrc);

	while(dat >= 0){
		fwrite(fdst, (char)dat);
		dat = fread(fsrc);
	}	

	fclose(fdst);
	fclose(fsrc);
	return 0;
}

//******************************************************
// ファイルを削除します
// エラーの時は、-1を返す
//******************************************************
int EEPFILE::fdelete(const char *filename)
{
	int sect = scanFilename(filename);

	if (sect == -1){
		return -1;
	}

	int next;
	while(true){
		next = Sect[sect] & 0x7F;		//次のセクタ読み込み
		Sect[sect] = EEP_EMPTY << 8;	//現セクタを空にする
		if (sect == next){				//最終セクタには自分のセクタ番号が書いてある
			break;
		}
		sect = next;
	}

	//FATデータをEEPROMに保存する
	saveFat();

	return 0;
}

//******************************************************
// 書き込み
// file->seek位置に指定量のデータを書き込みます
//******************************************************
int EEPFILE::fwrite(FILEEEP *file, char *arry, int *len)
{
	int mlen = 0;
	for(int i=0; i<*len; i++){
		if (fwrite(file, arry[i])==-1){
			*len = mlen;
			return -1;
		}
		mlen++;
	}
	return 1;
}

//******************************************************
// 書き込み
// file->seek位置に書き込みます
//******************************************************
int EEPFILE::fwrite(FILEEEP *file, char dat)
{
	int secadd = 0;

	//書き込むセクタを求めます
	int sect = getSect(file, &secadd);
	if (sect == -1){
		return -1;
	}

	//書き込みます
	int add = sect * EEPSECTOR_SIZE + secadd;
	epWrite( add, dat );

	//seekを1つ進めます
	file->seek++;

	//seek位置がfileSizeを超えた場合
	if (file->seek > file->filesize){
		file->filesize = file->seek;
	}
	return 1;
}

//******************************************************
// 読み込み
// file->seek位置を読み込みます
//******************************************************
int EEPFILE::fread(FILEEEP *file)
{
	//seek位置がfileSize以上の場合
	if (file->seek >= file->filesize){
		file->seek = file->filesize;
		return -1;
	}

	int secadd = 0;

	//読み込むセクタを求めます
	int sect = getSect(file, &secadd);
	//if (sect == -1){		//←ありえないので
	//	return -1;
	//}

	//読み込みます
	int add = sect * EEPSECTOR_SIZE + secadd;
	int ret = EEPROM.read( add );

	//seekを1つ進めます
	file->seek++;

	return ret;
}

//******************************************************
// ファイルを閉じます
//******************************************************
void EEPFILE::fclose(FILEEEP *file)
{
	DEBUG_PRINT("file->stasector", file->stasector);
	DEBUG_PRINT("file->filesize", file->filesize);
	DEBUG_PRINT("file->offsetaddress", file->offsetaddress);
	DEBUG_PRINT("file->seek", file->seek);

	int sect = file->stasector;
	if (sect < EEPSTASECT){	return;	}

	DEBUG_PRINT("fclose Sect[sect]", Sect[sect]);
	if(((Sect[sect] >> 10) & 0x3) == EEP_WRITE){
		//ファイルサイズを書き込みます
		DEBUG_PRINT("fclose", "EEP_WRITE");
		int add = file->stasector * EEPSECTOR_SIZE;
		epWrite(add + file->offsetaddress - 1, (file->filesize>>8) & 0xFF);
		epWrite(add + file->offsetaddress - 2, file->filesize & 0xFF);
	}

	int next;
	while(true){
		Sect[sect] = (Sect[sect] & 0xF3FF) | (EEP_CLOSE << 10);	//セクタにCloseフラグセット
		next = Sect[sect] & 0x7F;		//次のセクタ読み込み
		if (sect == next){				//最終セクタには自分のセクタ番号が書いてある
			break;
		}
		sect = next;
	}

	//FATデータをEEPROMに保存する
	saveFat();

	file->filesize = 0;
	file->offsetaddress = 0;
	file->seek = 0;
	file->stasector = -1;
}

//******************************************************
// ファイルの存在を調べます
// 0:無し, 1:在り
//******************************************************
int EEPFILE::fexist(const char *filename)
{
	//ファイルを探します
	int sect = scanFilename(filename);

	if (sect == -1){
		return 0;
	}
	return 1;
}

//******************************************************
// ファイルの終端したらtrueを返します
//******************************************************
bool EEPFILE::fEof(FILEEEP *file)
{
	return (file->seek >= file->filesize)?true:false;
}

//******************************************************
// セクタに保存されているファイル名を返します
// セクタにファイル名が無ければ filename[0]=0 が返ります
// 戻り値はファイルサイズです
//******************************************************
int EEPFILE::fdir(int sect, char *filename)
{
	int ret = 0;
	//ファイル先頭セクタか
	if(((Sect[sect] >> 8) & 0x3) == EEP_TOP){
		getFilename(sect, filename);
		ret = ffilesize( filename );
	}
	else{
		filename[0] = 0;
	}
	return ret;
}

//******************************************************
// ファイルサイズを返します
//******************************************************
int EEPFILE::ffilesize(const char *filename)
{
	int ret = 0;
	int sect = scanFilename(filename);
	int len = strlen(filename);

	if (sect != -1){
		int add = sect * EEPSECTOR_SIZE;
		ret = (EEPROM.read( add + len + 1 ) & 0xFF) + ((EEPROM.read( add + len + 2 ) & 0xFF) << 8);
	}
	return ret;
}


//***
//  Private methods
//***

//*********
// ファイル名を取得する
// ファイル文字数(バイト)が返る。最後の0x00は数えない。
//*********
int EEPFILE::getFilename(int sect, char *filename)
{
	//DEBUG_PRINT("getFilename sect", sect);

	if (sect < EEPSTASECT){ return 0; }

	unsigned long add = sect * EEPSECTOR_SIZE;
	int len = 0;
	for (int i = (int)add; i <= (int)add + EEPFILENAME_SIZE; i++){
		filename[len] = (char)EEPROM.read(i);

		if (filename[len] == 0){
			//DEBUG_PRINT("getFilename filename", filename);
			//DEBUG_PRINT("getFilename len", len);
			return len;
		}
		len++;
	}

	//DEBUG_PRINT("getFilename filename", filename);
	return 0;
}

//*********
// ファイル名を探す(ファイル名は31バイトまで)
// 見つかったセクタ番号を返す
// 見つからないときは、-1を返す
//*********
int EEPFILE::scanFilename(const char *filename)
{
	//DEBUG_PRINT("scanFilename", filename);

	int flen = strlen((const char*)filename);
	if(flen >= EEPFILENAME_SIZE){ return -1; }

	char fn[EEPFILENAME_SIZE];
	int len = 0;
	int ret = -1;
	for (int i = EEPSTASECT; i <= EEPENDSECT; i++){

		//DEBUG_PRINT("scanFilename i", i);

		if (((Sect[i]>>8) & 0x3) == EEP_TOP){

			len = getFilename(i, fn);
			if (flen == len){
				ret = i;
				for (int j = 0; j < len; j++){
					if (filename[j] != fn[j]){
						ret = -1;
						break;
					}
				}
			}
			if (ret != -1){	break;	}
		}
	}

	//DEBUG_PRINT("scanFilename ret", ret);
	return ret;
}

//*********
// 書き込まれていないブロックを探す
// ブロック番号を返す
// 全て書き込まれているときは、-1を返す
//*********
int EEPFILE::scanEmptySector(int start)
{
	int s0 = EEPSTASECT;
	if (start >= EEPSTASECT && start <= EEPENDSECT){
		s0 = start;
	}

	for (int i = s0; i <= EEPENDSECT; i++){
		if (((Sect[i] >> 8) & 0x3) == EEP_EMPTY){
			return i;
		}
	}

	for (int i = EEPSTASECT; i < s0; i++){
		if (((Sect[i] >> 8) & 0x3) == EEP_EMPTY){
			return i;
		}
	}
	return -1;
}

//*********
// ファイルポインタの書き込みを行う
//*********
void EEPFILE::setFile( FILEEEP *file, const char *filename, int sect, int mode)
{
	int add = sect * EEPSECTOR_SIZE;
	int len = strlen(filename);
	//int a1;

	//セクタ配列をセットします(自分自身をセット)
	Sect[sect] = sect & 0x7F;		//とりあえずトップセクタ番号をセットする
	Sect[sect] |= EEP_TOP << 8;		//ファイルトップフラグ
	Sect[sect] |= mode << 10;		//ファイル使用中フラグ

	//ファイル名を書き込む
	for( int i=0; i<len; i++){
		epWrite( add + i, filename[i] );
	}
	epWrite( add + len, 0 );

	//ファイルサイズ 0 セット
	epWrite( add + len + 1, 0 );
	epWrite( add + len + 2, 0 );

	file->filesize = 0;
	file->stasector = sect;
	file->offsetaddress = len + 3;
	file->seek = 0;
}

//*********
// FATをEEPROMに保存します
//*********
void EEPFILE::saveFat(void)
{	
	//int a1, a2;
	for( int i=0; i<EEPSECTORS; i++){
		// 11,12ビット目は0にして保存する 1111 0011 1111 1111
		//a1 = (Sect[i] & (~(3 << 10))) & 0xFF;
		//a2 = ((Sect[i] & (~(3 << 10)))>>8) & 0xFF;
		epWrite( EEPFAT_START + i*2, (Sect[i] & (~(3 << 10))) & 0xFF);
		epWrite( EEPFAT_START + i*2 + 1, ((Sect[i] & (~(3 << 10)))>>8) & 0xFF);
	}
}

//*********
// file->seekが示す場所のセクタを求めます
// そのセクタのアドレスもaddに入れて返します
// 全て書き込まれているときは、-1を返す
//*********
int EEPFILE::getSect(FILEEEP *file, int *add)
{
	if(file->stasector < EEPSTASECT){
		return -1;
	}

	//何セクタ分かを求める
	int n = (file->offsetaddress + file->seek) / EEPSECTOR_SIZE;
	*add = (file->offsetaddress + file->seek) % EEPSECTOR_SIZE;

	int sect = file->stasector;

	//DEBUG_PRINT("getSect n", n);
	//DEBUG_PRINT("getSect add", *add);
	//DEBUG_PRINT("getSect file->filesize", file->filesize);
	//DEBUG_PRINT("getSect file->seek", file->seek);

	//セクタを追っかける
	for(int i=0; i<n; i++){
		sect = Sect[sect] & 0x7F;
	}
	//DEBUG_PRINT("getSect sect 1", sect);
	//DEBUG_PRINT("getSect EEP_WRITE", (Sect[file->stasector] >> 10) & 0x3);

	//書き込みオープンのとき
	if(((Sect[file->stasector] >> 10) & 0x3) == EEP_WRITE){
		//割り切れているときは、
		if(*add == 0){

			//DEBUG_PRINT("getSect file->stasector", file->stasector);
			//DEBUG_PRINT("getSect file->filesize", file->filesize);
			//DEBUG_PRINT("getSect file->offsetaddress", file->offsetaddress);
			//DEBUG_PRINT("getSect file->seek", file->seek);

			//割り切れているのに、Sect[sect]にある次のセクタ番号が自分自身であったら、
			if (file->seek >= file->filesize){
				//新規セクタを用意する
				int newsect = scanEmptySector((int)millis() & 0x3F);	//(int)millis()&0x3Fは、乱数の要素を含ませているセクタの平滑化利用(ウェアレベリングを考慮)
				if (newsect == -1){
					//ファイルがいっぱいだった
					return -1;
				}
				//次のセクタを入れる
				Sect[sect] = (Sect[sect] & 0xFF80) | newsect;

				DEBUG_PRINT("getSect newsect", newsect);

				//セクタ配列をセットします
				Sect[newsect] = newsect & 0x7F;			//最後尾なので自分自身をセット
				Sect[newsect] |= EEP_USED << 8;			//ファイルユーズフラグ
				Sect[newsect] |= EEP_WRITE << 10;		//ファイル使用中フラグ
				sect = newsect;
			}
		}
	}
	return sect;
}

//*********
// EEPROMに書き込みます
// エラーのときは-1を返す
//*********
int EEPFILE::epWrite(unsigned long addr,unsigned char data)
{
	if (data != EEPROM.read(addr)){
		return (EEPROM.write(addr, data) == FLASH_SUCCESS ? 1 : -1);
	}
	return 1;
}

//*********
// EEPROMが初期化状態(FFFFで埋まっている)であれば、0を返します
//*********
int EEPFILE::isReady()
{
unsigned short sect;

	for (int i=0; i<EEPSECTORS; i++){
		sect = EEPROM.read( EEPFAT_START + i*2 ) + (EEPROM.read( EEPFAT_START + i*2 + 1 )<<8);
		
		if(sect != 0xFFFF){ return 1; }
	}
	return 0;
}
