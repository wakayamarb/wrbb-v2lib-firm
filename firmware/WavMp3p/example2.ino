/*
 * gr_sketch.cpp - wavmp3p library
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
/* This template is based on GR-CITRUS Sketch Template V1.00 */
/* Oct. 20th 2016, Yuuki Okamiya: modified using MTU1 for GR-CITRUS.*/

#include <Arduino.h>
#include <Utilities.h>
#include <MsTimer2.h>
#include <WavMp3p.h>
#include <SD.h>

#define LED_ON		HIGH
#define LED_OFF		LOW

#define SW_PAUSE	5
#define SW_SKIP		4
#define SW_ON		0
#define SW_OFF		1

WavMp3p wavmp3p(44100);
File root;

static
void cyclic_handler(void);

void setup()
{
	Serial.begin(9600);
	Serial.println("wav & mp3 player");

	pinMode(PIN_LED0, OUTPUT);
	digitalWrite(PIN_LED0, LED_ON);

	while(!SD.begin())
	{
		Serial.println("Card failed, or not present.");
		digitalWrite(PIN_LED0, LED_OFF);
		delay(1000);
		digitalWrite(PIN_LED0, LED_ON);
	}

	pinMode(SW_PAUSE, INPUT_PULLUP);
	pinMode(SW_SKIP, INPUT_PULLUP);

	MsTimer2::set(100, cyclic_handler);
	MsTimer2::start();
}

// This sample plays the .wav or .mp3 in a card in order.
// カードの.wav または .mp3 ファイルを見つけた順に再生するサンプル
char path[1024][8+1+3+1];

void printDirectory(File dir, int numTabs)
{
	while(true)
	{
		File entry = dir.openNextFile();

		if(!entry)
		{
			return;
		}

		if(entry.isDirectory())
		{
			strcpy(path[numTabs], entry.name());
			printDirectory(entry, numTabs + 1);
		}
		else
		{
			char fpath[1024] = "";
			for(uint8_t i=0; i< numTabs; i++)
			{
				strcat(fpath, path[i]);
				strcat(fpath, "/");
			}
			strcat(fpath, entry.name());
			Serial.print(fpath);

			char *res = wavmp3p.play(fpath);
			if(res)Serial.println(res);
		}
	}
}

void loop()
{
	root = SD.open("/");
	printDirectory(root, 0);
	Serial.println("done!");
}

// The below handler enables to skip or stop.
// 一時停止，スキップ機能のサンプル
static
void cyclic_handler()
{
	static int ct_pause = 0;

	if(SW_ON == digitalRead(SW_PAUSE)){
		if(ct_pause == 2){
			if(wavmp3p.read_pause()){
				wavmp3p.pause(0);
				digitalWrite(PIN_LED0, LED_ON);
			}else{
				wavmp3p.pause(1);
				digitalWrite(PIN_LED0, LED_OFF);
			}
			ct_pause++;
		}else if(ct_pause < 2){
			ct_pause++;
		}
	}else{
		ct_pause = 0;
	}

	static int ct_skip = 0;

	if(SW_ON == digitalRead(SW_SKIP)){
		if(ct_skip == 2){
			wavmp3p.skip();
			digitalWrite(PIN_LED0, LED_ON);
			ct_skip++;
		}else if(ct_skip < 2){
			ct_skip++;
		}
	}else{
		ct_skip = 0;
	}
}
