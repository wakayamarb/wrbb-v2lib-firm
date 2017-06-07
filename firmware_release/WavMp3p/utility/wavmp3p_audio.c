/*
 * wavmp3p_audio.c - wavmp3p library
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

#include <iodefine.h>
#include "wavmp3p_audio.h"
#include "wavmp3p_dma.h"
#include "wavmp3p_pwm.h"

static int ch = 0;
static int databit = 0;
static int pwm_bit = 0;
static unsigned long pwm_clk = 0;
static unsigned long pwm_count;
static unsigned long sf = 0;

/* パラメータセット */
void wavmp3p_audio_set(unsigned long p_sf, int p_databit, int p_ch)
{
	ch = p_ch;
	databit = p_databit;
	sf = p_sf;

	pwm_clk = wavmp3p_pwm_clk();
	pwm_count = (unsigned short)(pwm_clk / sf );
    MTU1.TGRA = pwm_count - 1;
    MTU1.TGRB = pwm_count / 2 - 1;
	int n;
	for(n = 15; 0 <= n; n--)
	{
		if(pwm_count & (1 << n))
		{
			break;
		}
	}

	pwm_bit = n;
	return;
}

unsigned short ringbuf[RINGBUF_SIZE * 2 * 2];	//4Kバイトにアラインするため2倍の領域を確保
unsigned short *ringbuf_p;						//4Kバイトでアラインしたバッファの先頭アドレス
static int decode_putp = 0;

#define RINGBUF_SIZE_MAX (RINGBUF_SIZE)
#define p_inc(p, max) (((p + 1) >= max) ? 0 : (p + 1))

#define GET_NG 0
#define GET_OK 1

typedef short t_wav_data;
#define WAV2DUTY(w) (((w + (1L << (databit - 1))) >> (databit - pwm_bit)) + (pwm_count - (1L << pwm_bit)) / 2)

/* リングバッファに波形データを入れる */
int decode_put(t_wav_data left, t_wav_data right)
{
	unsigned int putp = p_inc(decode_putp, RINGBUF_SIZE_MAX);
	int r = PUT_NG;

	// 空きがあるか
	if(putp != decode_getp())
	{
        *(ringbuf_p + decode_putp) = WAV2DUTY(left);

		decode_putp = putp;
		r = PUT_OK;
	}
	return r;
}

void wavmp3p_put(void *read_buffer, int size)
{
	int n;
	t_wav_data audio_l, audio_r, *data = read_buffer;

	for(n = 0; n < size / ch / sizeof(t_wav_data); n++)
	{
		audio_l = *data++;
		audio_r = (2 == ch) ? *data++ : audio_l;

		// リングバッファにデータを書きこむ
		// 空きが無ければ待つ
		while (PUT_OK != decode_put(audio_l, audio_r));
	}
	return;
}

void wavmp3p_audio_init(void)
{
	int n;
	unsigned short *rp;

	ringbuf_p = (unsigned short *)(((int)ringbuf + 0x1000) & 0xfffff000);	//4Kバイトでアライン
	rp = ringbuf_p;

	for(n = 0; n < RINGBUF_SIZE; n++)
	{
		*rp++ = WAV2DUTY(0);
		*rp++ = WAV2DUTY(0);
	}
	return;
}

//リングバッファが空か
int decode_empty(void)
{
	int r = EMPTY_NG;

	if(decode_putp == decode_getp())
	{
		r = EMPTY_OK;
	}
	return r;
}

//出力を無音に
void audio_stop(void)
{
	int n;
	unsigned short *rp = ringbuf_p;

	//リングバッファ内のデータの出力が終わるまで待つ
	while(EMPTY_NG == decode_empty());

	for(n = 0; n < RINGBUF_SIZE; n++)
	{
		*rp++ = WAV2DUTY(0);
		*rp++ = WAV2DUTY(0);
	}

	return;
}
