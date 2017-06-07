/*
 * WavMp3p.h - wavmp3p library
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

#ifndef __WAVMP3P_H__
#define __WAVMP3P_H__

class WavMp3p
{
public:
	WavMp3p(unsigned long sf);
	char *play(const char* filename);
	bool read_pause(void);
	void pause(int command);
	void skip(void);
	void init(unsigned long sf);
};

#endif
