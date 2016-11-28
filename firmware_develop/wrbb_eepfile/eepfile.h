/*
 * EEPROMをファイルのように使うクラス
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#ifndef _EEPFILE_H_
#define _EEPFILE_H_ 1

#include <Arduino.h>

#define EEPFILENAME_SIZE	32

#define EEP_CLOSE	0		//オープンしていない
#define EEP_READ	1		//READオープン
#define EEP_WRITE	2		//WRITEオープン
#define EEP_APPEND	3		//APPENDオープン

//EEPファイル構造体
typedef struct {
	short stasector;
	unsigned short filesize;
	unsigned short offsetaddress;
	unsigned short seek;
} FILEEEP;

enum EEPFILE_seek { EEP_SEEKTOP, EEP_SEEKCUR, EEP_SEEKEND };

class EEPFILE
{
  public:
	void begin(void){	begin(0);	}
	void format(void){	begin(1);	}
	void begin(int clear);
	int fopen(FILEEEP *file, const char *filename, char mode);
	int fdelete(const char *filename);
	int fcopy(const char *srcfilename, const char *dstfilename);
	int ffilesize(const char *filename);
	int fseek(FILEEEP *file, int offset, int origin);
	int fwrite(FILEEEP *file, char dat);
	int fwrite(FILEEEP *file, char *arry, int *len);
	int fread(FILEEEP *file);
	void fclose(FILEEEP *file);
	int fexist(const char *filename);
	bool fEof(FILEEEP *file);
	int fdir(int sect, char *filename);
	void viewFat(void);
	void viewSector(int sect);

  private:
	int getFilename(int sect, char *filename);
	int scanFilename(const char *filename);
	int scanEmptySector(int start);
	void setFile( FILEEEP *file, const char *filename, int sect, int mode);
	void saveFat(void);
	int getSect(FILEEEP *file, int *add);
	int epWrite(unsigned long addr,unsigned char data);
	int isReady();
};

extern EEPFILE	EEP;

#endif // _EEPFILE_H_

