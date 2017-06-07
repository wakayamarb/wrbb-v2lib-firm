/*
 * SDカード関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#ifndef _SSDCARD_H_
#define _SSDCARD_H_  1

#define SD_CLASS	"SD"

//**************************************************
// ライブラリを定義します
//**************************************************
int sdcard_Init(mrb_state *mrb);

//**************************************************
// 日付と時刻を返すコールバック関数
//**************************************************
void SD_DateTime(uint16_t *date, uint16_t *time);

//**************************************************
// SDカードライブラリを初期化します
// filenameが指定されていれば、そのファイルが存在すれば成功を返します
// 失敗 0, 成功 1
//**************************************************
int SD_init(char *filename);

//**************************************************
// SDカードのファイルをフラッシュメモリにコピーします
// 失敗 0, 成功 1
//**************************************************
int SD2EEPROM(const char *sdfile, const char *eepfile);

#endif // _SSDCARD_H_