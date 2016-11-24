/*
 * wavmp3p_play_wav.c - wavmp3p library
 * Copyright (c) 2015 Mitsuhiro Matsuura.  All right reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <string.h>

#include "wavmp3p_audio.h"
#include "wavmp3p_ctrl.h"
#include "wavmp3p_play.h"
#include "wavmp3p_play_wav.h"

static char message[256];

// WAVファイル再生処理
char *wavmp3p_play_wav(const char *name)
{
	FRESULT res;
	UINT size;
	unsigned long chunk_size;
	short read_buffer[2*1024];

#define filename name

	res = f_open(filename);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_open:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;

	// "RIFF"チェック
	res = f_read((void *)read_buffer, 8, &size);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;
	if(0 != strncmp((char *)read_buffer, "RIFF", 4))goto play_return;

	// "WAVE"チェック
	res = f_read((void *)read_buffer, 4, &size);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;
	if(0 != strncmp((char *)read_buffer, "WAVE", 4))goto play_return;

	// "fmt "チェック
	res = f_read((void *)read_buffer, 4, &size);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;
	if(0 != strncmp((char *)read_buffer, "fmt ", 4))goto play_return;

	// fmtチャンクデータサイズ取得
	res = f_read((void *)read_buffer, 4, &size);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;
	unsigned long n = *(unsigned long *)read_buffer;
	if(0 == n)goto play_return;

	res = f_read((void *)read_buffer, n, &size);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;

	// チャンネル数チェック
	// 2(ステレオ) と 1(モノラル) に対応
	int ch = *((unsigned short *)read_buffer + 1);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, ch:%d\r\n", __FILE__, __LINE__, ch);
	if((2 != ch) && (1 != ch))goto play_return;

	// サンプリング周波数
	unsigned long sf = *((unsigned long *)read_buffer + 1);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, sf:%d\r\n", __FILE__, __LINE__, (int)sf);

	// ビット数チェック
	// 16bit のみに対応
	int databit = *((unsigned short *)read_buffer + 7);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, bit:%d\r\n", __FILE__, __LINE__, databit);
	if(16 != databit)goto play_return;

	// "data"チェック
	res = f_read((void *)read_buffer, 4, &size);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;
	if(0 != strncmp((char *)read_buffer, "data", 4))goto play_return;

	// 波形データサイズ取得
	res = f_read((void *)read_buffer, 4, &size);
	sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;
	chunk_size = *(unsigned long *)read_buffer;
	if(0 == chunk_size)goto play_return;

	// 読み出したパラメータをセット
	wavmp3p_audio_set(sf, databit, ch);
	wavmp3p_audio_init();

	// 再生ループ
	while(0 == wavmp3p_ctrl() && chunk_size > 0)
	{
		// 波形データをローカルバッファに読み出す
		res = f_read((void *)read_buffer, sizeof(read_buffer), &size);
		sprintf(message, "wavmp3p_play_wav error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
		if(res != FR_OK || 0 == size) goto play_return;

		// ローカルバッファからリングバッファにコピー
		wavmp3p_put(read_buffer, (int)size);
		chunk_size -= size;
	}

	// 正常終了時はメッセージをクリア
	message[0] = '\0';

play_return:
	f_close();

	// 波形出力終了待ち
	audio_stop();
	return message;
}
