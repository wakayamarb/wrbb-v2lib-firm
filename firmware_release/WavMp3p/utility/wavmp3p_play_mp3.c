/*
 * wavmp3p_play_mp3.c - wavmp3p library
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
#include "wavmp3p_play_mp3.h"

extern int decode(unsigned char const *start, unsigned long length);

#define MP3_WORK_SIZE (1152*2*2)
static unsigned char mp3_work[MP3_WORK_SIZE];
static unsigned char *next_header;

static char message[256];

FRESULT get_header(void)
{
	FRESULT res;
	UINT size;

	res = f_read((void*)mp3_work, 10, &size);
	sprintf(message, "get_header error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto get_header_return;

	if(0 == strncmp((char*)mp3_work, "ID3", 3))
	{
		UINT id3_size =
			((*(mp3_work + 6) & 0x7f) << 21) +
			((*(mp3_work + 7) & 0x7f) << 14) +
			((*(mp3_work + 8) & 0x7f) << 7) +
			((*(mp3_work + 9) & 0x7f) << 0);

		res = f_read((void*)(mp3_work + 10), id3_size, &size);	/* ID3ver2タグを読み飛ばす */
		sprintf(message, "get_header error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
		if(res != FR_OK) goto get_header_return;

		res = get_header();	/* もう一度 */
		goto get_header_return;
	}

	if(0 == strncmp((char*)mp3_work, "RIFF", 4))
	{
		res = f_read((void*)(mp3_work + 10), 10, &size);	/* RIFF MP3ヘッダを読み飛ばす */
		sprintf(message, "get_header error file[%s] line:%d, f_read:%d\r\n", __FILE__, __LINE__, res);
		if(res != FR_OK) goto get_header_return;

		res = get_header();	/* もう一度 */
		goto get_header_return;
	}

get_header_return:
	return res;
}

// MP3ファイル再生処理
char *wavmp3p_play_mp3(const char *name)
{
	FRESULT res;

	next_header = 0;

#define filename name

	res = f_open(filename);
	sprintf(message, "wavmp3p_play_mp3 error file[%s] line:%d, f_open:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;

	res = get_header();
	if(res != FR_OK) goto play_return;

	sprintf(message, "wavmp3p_play_mp3 error file[%s] line:%d, %d\r\n", __FILE__, __LINE__, res);
	if(mp3_work[0] != 0xff) goto play_return;
	sprintf(message, "wavmp3p_play_mp3 error file[%s] line:%d, %d\r\n", __FILE__, __LINE__, res);
	if((mp3_work[1] & 0xf0) != 0xf0) goto play_return;
	next_header = mp3_work;

	// サンプリング周波数
	unsigned long sf, sf_list[] = {44100, 48000, 32000, 0};
	sf = sf_list[(mp3_work[2] >> 2) & 0x03];
	sprintf(message, "wavmp3p_play_mp3 error file[%s] line:%d, sf:%d\r\n", __FILE__, __LINE__, (int)sf);
	if(0 == sf) goto play_return;

	// チャンネル数チェック
	// 2(ステレオ) と 1(モノラル) に対応
	int ch, ch_list[] = {2, 2, 2, 1};
	ch = ch_list[(mp3_work[3] >> 6) & 0x03];
	sprintf(message, "wavmp3p_play_mp3 error file[%s] line:%d, ch:%d\r\n", __FILE__, __LINE__, ch);
	if(1 != ch && 2 != ch) goto play_return;

	// 読み出したパラメータをセット
	wavmp3p_audio_set(sf, 16, ch);
	wavmp3p_audio_init();

	decode((unsigned char const *)mp3_work, (unsigned long)MP3_WORK_SIZE);
	audio_stop();

	res = f_close();
	sprintf(message, "wavmp3p_play_mp3 error file[%s] line:%d, f_close:%d\r\n", __FILE__, __LINE__, res);
	if(res != FR_OK) goto play_return;

	// 正常終了時はメッセージをクリア
	message[0] = '\0';

play_return:
	f_close();

	// 波形出力終了待ち
	audio_stop();
	return message;
}

void read_frame(unsigned int N)
{
	FRESULT res;
	unsigned int size;

	res = f_read((void*)(mp3_work + 10), N, &size);	/* 10バイト目以降のNバイトをワークメモリに読み込む */
	if(res != FR_OK) goto read_frame_error;
	if(N != size) goto read_frame_error;

	next_header = mp3_work + N;
	return;

read_frame_error:
	*(int*)mp3_work = 0;
	next_header = mp3_work;
	return;
}

void copy_next_header(void)
{
	memcpy(mp3_work, next_header, 10);	/* 先読みした「次のフレームのヘッダから10バイト」を先頭にコピー */
	return;
}

unsigned char* get_next_frame(void)
{
	return (unsigned char*)mp3_work;
}
