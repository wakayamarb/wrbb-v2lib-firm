/*
 * readme.txt - wavmp3p library
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

●概要

	本プログラムは GR-CITRUS を使用してSDメモリカードのWAV/MP3ファイルを
	再生するアプリケーションです．
	「がじぇっとるねさす」が提供するクラウドWEBコンパイラにアップロードしてビルドします．
	ライセンスはGPL2です．

●対応WAVファイル

	サンプリング周波数	32kHz,44.1kHz,48kHz（48MHzを分周して生成するため誤差あり）
	チャネル数			２（ステレオ）または１（２つの出力端子に同じ信号を出力）
	ビット数			１６
	ファイル名			８文字＋拡張子"WAV"

●対応MP3ファイル

	サンプリング周波数	32kHz,44.1kHz,48kHz（48MHzを分周して生成するため誤差あり）
	チャネル数			２（ステレオ）または１（２つの出力端子に同じ信号を出力）
	ビット数			１６
	ファイル名			８文字＋拡張子"MP3"

●注意事項

	音声出力はPWMのパルス波形です．
	PWMの周波数はサンプリング周波数です．サンプリング周波数が可聴域だと「ピー音」が出ます．
	サンプリング周波数，チャネル数，ビット数，PWM周波数の組合せによっては処理が間に合わない場合があります．
	接続するオーディオ機器によってはノイズが発生したり，最悪の場合は機器を破壊する可能性があります．
	SDメモリカードは相性があります．動作しない場合は他のメーカー品を試してください．

●ファイル構成
  （割愛）

●ピン接続

	－－－－－－－－－－－－－－－－－－
	｜GR-CITRUS         ｜             ｜
	｜－－－－－－－－－＋－－－－－－ ｜
	｜                  ｜音声出力     ｜
	｜IO0/P20/MTIOC1A   ｜ 左チャネル  ｜
	－－－－－－－－－－－－－－－－－－

●使用タイマリソース

	マルチファンクション・タイマ・ユニット１

●使用しているオープンソース

	libmad

		http://www.underbit.com/products/mad/

以上
