/*
 * wavmp3p_dma.c - wavmp3p library
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

void wavmp3p_dma_init(void)
{
    DMAC0.DMSAR = (void *)ringbuf_p;    //転送元アドレスレジスタ
    DMAC0.DMDAR = (void *)&MTU1.TGRB;   //転送先アドレスレジスタ
    DMAC0.DMCRA = 0;                    //転送カウントレジスタ
    DMAC0.DMCRB = 0;                    //転送カウントレジスタ
    DMAC0.DMTMD.BIT.DCTG = 1;           //転送要求選択ビット＝周辺モジュールからの割り込み
//    DMAC0.DMTMD.BIT.SZ = 2;             //データ転送サイズ＝32ビット転送
    DMAC0.DMTMD.BIT.SZ = 1;             //データ転送サイズ＝16ビット転送
//  DMAC0.DMTMD.BIT.DTS = 1;            //リピート領域選択＝転送元
    DMAC0.DMTMD.BIT.DTS = 2;            //リピート領域選択
    DMAC0.DMTMD.BIT.MD = 0;             //転送モード設定＝ノーマル転送
//  DMAC0.DMTMD.BIT.MD = 1;             //転送モード設定＝リピート転送
    DMAC0.DMAMD.BIT.DARA = 0;           //拡張リピートを使用しない
    DMAC0.DMAMD.BIT.DM = 0;             //転送先アドレス更新モード＝アドレス固定
//  DMAC0.DMAMD.BIT.SARA = 0;           //拡張リピートを使用しない
    DMAC0.DMAMD.BIT.SARA = 11;          //下位11ビット(4Kバイト)を拡張リピート
    DMAC0.DMAMD.BIT.SM = 2;             //転送元アドレス更新モード＝アドレス加算
    DMAC0.DMCSL.BIT.DISEL = 0;          //インタラプト選択＝転送開始後に起動要因となった割り込みフラグを"0"クリアする
    ICU.DMRSR0 = 148u;                  //DMA起動要求選択
    DMAC0.DMCNT.BIT.DTE = 1;            //DMA転送を許可

    DMAC.DMAST.BIT.DMST = 1;            //DMA動作許可ビット＝DMA起動を禁止許可
	return;
}

int decode_getp(void)
{
    return ((unsigned int)DMAC0.DMSAR - (unsigned int)ringbuf_p) / sizeof(unsigned short);
}
