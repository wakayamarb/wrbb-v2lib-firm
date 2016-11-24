/*
 * WavMp3p.cpp - wavmp3p library
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

#include "WavMp3p.h"

#include <string.h>


extern "C" {
#include "wavmp3p_audio.h"
#include "wavmp3p_ctrl.h"
#include "wavmp3p_init.h"
#include "wavmp3p_play_mp3.h"
#include "wavmp3p_play_wav.h"
};

WavMp3p::WavMp3p(unsigned long sf)
{
	wavmp3p_init(sf);
}

void WavMp3p::init(unsigned long sf)
{
	wavmp3p_init(sf);
}

char *WavMp3p::play(const char* filename)
{
	static char message[] = "\0";

	if((NULL != strstr(filename, ".mp3"))
	|| (NULL != strstr(filename, ".MP3")))
	{
		return wavmp3p_play_mp3(filename);
	}
	else
	if((NULL != strstr(filename, ".wav"))
	|| (NULL != strstr(filename, ".WAV")))
	{
		return wavmp3p_play_wav(filename);
	}
	return message;
}

bool WavMp3p::read_pause(void)
{
	return wavmp3p_read_pause() ? true : false;
}

void WavMp3p::pause(int command)
{
	return wavmp3p_pause(command);
}

void WavMp3p::skip(void)
{
	wavmp3p_skip();
}
