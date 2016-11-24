/*
 * wavmp3p_audio.h - wavmp3p library
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

#define PUT_NG 0
#define PUT_OK 1

#define EMPTY_NG 0
#define EMPTY_OK 1

#define RINGBUF_SIZE 1024	//DMAの拡張リピートサイズ
extern unsigned short ringbuf[];
extern unsigned short *ringbuf_p;
extern void wavmp3p_audio_init(void);
extern void wavmp3p_audio_set(unsigned long sf, int databit, int ch);
extern int decode_put(short left, short right);
extern void wavmp3p_put(void *read_buffer, int size);
extern int decode_empty(void);
extern void audio_stop(void);
