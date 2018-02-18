/*
 * wrbb.h
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#ifndef _WRBB_H_
#define _WRBB_H_  1

//#define	DEBUG		// Define if you want to debug

#ifdef DEBUG
#  define DEBUG_PRINT(m,v)    { Serial.print("** "); Serial.print((m)); Serial.print(":"); Serial.println((v)); }
#else
#  define DEBUG_PRINT(m,v)    // do nothing
#endif

#define RUBY_CODE_SIZE (1024 * 8)		//サイズ制限は無くしたが、8kb以下であればmallocしなくてもいいような仕組みを持たせている

//バイトコードバージョンを定義します
#define BYTE_CODE2	2
#define BYTE_CODE3	3
#define BYTE_CODE4	4

//ファームウェアのバージョンを定義します
#define MASTER		1000
#define JAM			1001
#define SDBT		1002
#define SDWF		1003
#define CITRUS		1004
//WRBB4は、Wakayama.rbボードのARIAD4基板でV2ファーム(MP3抜き)を設定するものです。ボード配線BOARD_P06となります。
#define WRBB4		1005

//基板の設計バージョンを定義します
#define BOARD_GR	0
#define BOARD_P01	1
#define BOARD_P02	2
#define BOARD_P03	3
#define BOARD_P04	4
#define BOARD_P05	5
#define BOARD_P06	6

//RX600シリーズの型番を定義します
#define CPU_RX63NB	128		//メモリ128KB 128KBを選択した場合は、HardwareSerial.h の /*GRSAKURA*/ #define SERIAL_BUFFER_SIZE 256 にしてください。
#define CPU_RX631F	256		//メモリ256KB

//モーター制御クラスを追加したもの
//#define SAMBOUKAN

//バージョンと日付
#define WRBB_VERSION	"-2.42(2018/2/18)"

//バイトコードフォーマットの設定
//#define BYTECODE	BYTE_CODE2
//#define BYTECODE	BYTE_CODE3
#define BYTECODE	BYTE_CODE4
#if BYTECODE == BYTE_CODE2
#	define BYTECODE_TEXT	"f2"
#elif BYTECODE == BYTE_CODE3
#	define BYTECODE_TEXT	"f3"
#elif BYTECODE == BYTE_CODE4
#	define BYTECODE_TEXT	"f4"
#endif

//基板のタイプ設定
//#define BOARD	BOARD_GR
//#define BOARD	BOARD_P01
//#define BOARD	BOARD_P02
//#define BOARD	BOARD_P03
//#define BOARD	BOARD_P04
//#define BOARD	BOARD_P05
#define BOARD	BOARD_P06
#if BOARD == BOARD_GR || BOARD == BOARD_P01
#	define BOARDNUM	"1"
#elif BOARD == BOARD_P02
#	define BOARDNUM	"2"
#elif BOARD == BOARD_P03
#	define BOARDNUM	"3"
#elif BOARD == BOARD_P04
#	define BOARDNUM	"4"
#elif BOARD == BOARD_P05
#	define BOARDNUM	"5"
#elif BOARD == BOARD_P06
#	define BOARDNUM	""
#endif

//ファームウェア設定
//#define FIRMWARE	MASTER
//#define FIRMWARE	JAM
//#define FIRMWARE	SDBT
//#define FIRMWARE	SDWF
#define FIRMWARE	CITRUS
//#define FIRMWARE	WRBB4

#if BOARD == BOARD_GR
#	if FIRMWARE == MASTER
#		define FIRMWARE_TEXT	"SakuRuby"
#	elif FIRMWARE == JAM
#		define FIRMWARE_TEXT	"SakuraJam"
#	endif
#else
#	if FIRMWARE == MASTER
#		define FIRMWARE_TEXT	"ARIDA"
#	elif FIRMWARE == JAM
#		define FIRMWARE_TEXT	"UmeJam"
#	elif FIRMWARE == SDBT
#		define FIRMWARE_TEXT	"SDBT"
#	elif FIRMWARE == SDWF
#		define FIRMWARE_TEXT	"SDWF"
#	elif FIRMWARE == CITRUS
#		define FIRMWARE_TEXT	"CITRUS"
#	elif FIRMWARE == WRBB4
#		define FIRMWARE_TEXT	"WRBB4"
#	endif
#endif

//RX600シリーズの設定
//#define CPU		CPU_RX63NB
#define CPU		CPU_RX631F
#if CPU == CPU_RX63NB
#	define CPURAM	"(128KB)"		//メモリ128KB
#else
#	define CPURAM	"(256KB)"		//メモリ256KB
#endif

//License表示
#define LICENSE_MRUBY		"mruby is released under the MIT License."
#define LICENSE_MRUBYURL	"https://github.com/mruby/mruby/blob/master/MITL"

#if BOARD != BOARD_P06
	#define LICENSE_WRBB		"Wakayama-mruby-board is released under the MIT License."
	#define LICENSE_WRBBURL		"https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL"
#else
	#if FIRMWARE == CITRUS
		#define LICENSE_WRBB		"Circuit and firmware of GR-CITRUS is released under the MIT License."
		#define LICENSE_WRBBURL		"https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL"
	#else
		#define LICENSE_WRBB		"Wakayama-mruby-board is released under the MIT License."
		#define LICENSE_WRBBURL		"https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL"
	#endif
#endif

//#define PIN_LED		61				//LEDのピン番号 GR-SAKURAのV2ファームの場合
#define PIN_LED		33					//LEDのピン番号 GR-CITRUSのV2ファームの場合

#define	FILE_LOAD	PORT3.PIDR.BIT.B5		//PORT 3-5

#define XML_FILENAME  "wrbb.xml"
#define RUBY_FILENAME  "main.mrb"
#define RUBY_FILENAME_SIZE 32

#if BOARD == BOARD_GR || BOARD == BOARD_P02 || BOARD == BOARD_P03 || BOARD == BOARD_P04 || BOARD == BOARD_P05 || BOARD == BOARD_P06
	#define REALTIMECLOCK	1
#endif

#if BOARD == BOARD_GR
	#define RB_PIN0		0
	#define RB_PIN1		1
	#define RB_PIN18	18
	#define RB_PIN19	19
	#define RB_PIN2		2
	#define RB_PIN3		3
	#define RB_PIN4		4
	#define RB_PIN5		5
	#define RB_PIN6		6
	#define RB_PIN7		7
	#define RB_PIN8		8
	#define RB_PIN9		9
	#define RB_PIN10	10
	#define RB_PIN11	11
	#define RB_PIN12	12
	#define RB_PIN13	13
	#define RB_PIN14	14
	#define RB_PIN15	15
	#define RB_PIN16	16
	#define RB_PIN17	17

	#define RB_PIN20	20
	#define RB_PIN21	21
	#define RB_PIN22	22
	#define RB_PIN23	23
	#define RB_PIN24	24
	#define RB_PIN25	25

	#define RB_PIN26	26
	#define RB_PIN27	27
	#define RB_PIN30	30
	#define RB_PIN31	31

#elif BOARD == BOARD_P01
	#define RB_PIN0		1
	#define RB_PIN1		0
	#define RB_PIN18	22
	#define RB_PIN19	23
	#define RB_PIN2		8
	#define RB_PIN3		30
	#define RB_PIN4		31
	#define RB_PIN5		24
	#define RB_PIN6		26
	#define RB_PIN7		6
	#define RB_PIN8		7
	#define RB_PIN9		53
	#define RB_PIN10	10
	#define RB_PIN11	11
	#define RB_PIN12	12
	#define RB_PIN13	13
	#define RB_PIN14	14
	#define RB_PIN15	15
	#define RB_PIN16	16
	#define RB_PIN17	17

	#define RB_PIN20	33
	#define RB_PIN21	29
	#define RB_PIN22	5
	#define RB_PIN23	45
	#define RB_PIN24	54
	#define RB_PIN25	100

#elif BOARD == BOARD_P02
	#define RB_PIN0		1
	#define RB_PIN1		0
	#define RB_PIN18	30
	#define RB_PIN19	31
	#define RB_PIN2		22
	#define RB_PIN3		23
	#define RB_PIN4		8
	#define RB_PIN5		24
	#define RB_PIN6		26
	#define RB_PIN7		6
	#define RB_PIN8		7
	#define RB_PIN9		53
	#define RB_PIN10	10
	#define RB_PIN11	11
	#define RB_PIN12	12
	#define RB_PIN13	13
	#define RB_PIN14	14
	#define RB_PIN15	15
	#define RB_PIN16	16
	#define RB_PIN17	17

	#define RB_PIN20	33
	#define RB_PIN21	29
	#define RB_PIN22	5
	#define RB_PIN23	45
	#define RB_PIN24	54
	#define RB_PIN25	100

#elif BOARD == BOARD_P03 || BOARD == BOARD_P04 || BOARD == BOARD_P05 || (BOARD == BOARD_P06 && PIN_LED == 61)
	#define RB_PIN0		1
	#define RB_PIN1		0
	#define RB_PIN18	30
	#define RB_PIN19	31
	#define RB_PIN2		22
	#define RB_PIN3		23
	#define RB_PIN4		8
	#define RB_PIN5		24
	#define RB_PIN6		26
	#define RB_PIN7		6
	#define RB_PIN8		7
	#define RB_PIN9		53
	#define RB_PIN10	10
	#define RB_PIN11	11
	#define RB_PIN12	12
	#define RB_PIN13	13
	#define RB_PIN14	14
	#define RB_PIN15	15
	#define RB_PIN16	16
	#define RB_PIN17	17

	#define RB_PIN20	33
	#define RB_PIN21	59
	#define RB_PIN22	60
	#define RB_PIN23	5
	#define RB_PIN24	57
	#define RB_PIN25	29
	#define RB_PIN26	58
	#define RB_PIN27	56
//	#define RB_PIN28	PB3
//	#define RB_PIN29	PB5
	#define RB_PIN30	45
	#define RB_PIN31	54
//	#define RB_PIN32	EMLE
	#define RB_PIN33	PIN_LED

#elif BOARD == BOARD_P06
	#define RB_PIN0		0
	#define RB_PIN1		1
	#define RB_PIN18	18
	#define RB_PIN19	19
	#define RB_PIN2		2
	#define RB_PIN3		3
	#define RB_PIN4		4
	#define RB_PIN5		5
	#define RB_PIN6		6
	#define RB_PIN7		7
	#define RB_PIN8		8
	#define RB_PIN9		9
	#define RB_PIN10	10
	#define RB_PIN11	11
	#define RB_PIN12	12
	#define RB_PIN13	13
	#define RB_PIN14	14
	#define RB_PIN15	15
	#define RB_PIN16	16
	#define RB_PIN17	17

	#define RB_PIN20	20
	#define RB_PIN21	21
	#define RB_PIN22	22
	#define RB_PIN23	23
	#define RB_PIN24	24
	#define RB_PIN25	25
	#define RB_PIN26	26
	#define RB_PIN27	27
	#define RB_PIN28	28
	#define RB_PIN29	29
	#define RB_PIN30	30
	#define RB_PIN31	31
//	#define RB_PIN32	EMLE
	#define RB_PIN33	33

#endif

#define RB_LED	PIN_LED


#endif // _WRBB_H_
