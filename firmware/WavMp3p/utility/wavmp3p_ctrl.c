/*
 * wavmp3p_ctrl.c - wavmp3p library
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

#include "wavmp3p_audio.h"
#include "wavmp3p_ctrl.h"

// 一時停止
static volatile int pause = 0;
int wavmp3p_read_pause(void)
{
	return pause;
}

void wavmp3p_pause(int command)
{
	pause = command;
	return;
}

// 再生中断
static volatile int skip = 0;
void wavmp3p_skip(void)
{
	skip = 1;
	return;
}

// 再生制御
int wavmp3p_ctrl(void)
{
	if(pause)
	{
		audio_stop();

		// 一時停止用ループ
		while(pause && !skip);
		pause = 0;
	}

	// 中断
	if(skip)
	{
		skip = 0;
		return 1;
	}

	return 0;
}
