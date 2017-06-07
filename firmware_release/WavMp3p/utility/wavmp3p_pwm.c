/*
 * wavmp3p_pwm.c - wavmp3p library
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
#include "wavmp3p_pwm.h"

#define MTU_CLOCK 48000000

void wavmp3p_pwm_init(unsigned long sf)
{
    MSTP_MTU1 = 0;

    MTU.TSTR.BIT.CST1 = 0;          //カウント停止

    MTU1.TCR.BIT.TPSC = 0;          //タイマプリスケーラ選択:PCLK/1
    MTU1.TCR.BIT.CKEG = 0;          //クロックエッジ選択
    MTU1.TCR.BIT.CCLR = 1;          //カウンタクリア要因選択:TGRAのコンペアマッチ
    MTU1.TMDR.BIT.MD = 2;           //動作モード:PWMモード1
    MTU1.TIER.BIT.TGIEA = 1;        //TGR割り込み許可Aビット

    MTU1.TIOR.BIT.IOA = 1;         //I/OコントロールAビット:初期出力は0，コンペアマッチで0
    MTU1.TIOR.BIT.IOB = 2;         //I/OコントロールBビット:初期出力は0，コンペアマッチで1

    MTU1.TGRA = MTU_CLOCK/sf-1;
    MTU1.TGRB = MTU_CLOCK/sf/2-1;

    MTU.TSTR.BIT.CST1 = 1;          //カウント開始
	return;
}

unsigned long wavmp3p_pwm_clk(void)
{
	return MTU_CLOCK;
}
