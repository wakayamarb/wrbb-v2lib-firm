/*
 * wavmp3p_play.cpp - wavmp3p library
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

#include "wavmp3p_play.h"
#include "SD.h"

File file;

FRESULT wavmp3p_open(const char* filename)
{
	file = SD.open(filename);
	return (true == file) ? FR_OK : ~FR_OK;
}

FRESULT wavmp3p_read(void* buff, unsigned int btr, unsigned int* br)
{
	*br = file.read((void *)buff, (uint16_t)btr);
	return (0 < *br && btr >= *br) ? FR_OK : ~FR_OK;
}

FRESULT wavmp3p_close(void)
{
	file.close();
	return FR_OK;
}
