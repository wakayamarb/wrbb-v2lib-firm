/*
 * I2C通信関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>
#include <Wire.h>

#include <mruby.h>
#include <mruby/data.h>
#include <mruby/class.h>

#include "../wrbb.h"
#include "sKernel.h"

#define WIRE_MAX	6

TwoWire *RbWire[WIRE_MAX];		//0:Wire1, 1:Wire3, 2:Wire2, 3:Wire6 4:Wire7


//**************************************************
// I2cをラップしたI2c2クラス作成
//**************************************************
class I2c2 {
public:
	I2c2(int num){
		num_ = num;

		//ソフトI2Cの場合は18PINと19PINをOUTPUTにする
		if(num == 0){
			pinMode(RB_PIN18, OUTPUT);
			pinMode(RB_PIN19, OUTPUT);
		}
		RbWire[num_]->begin();
	}
	virtual ~I2c2() {
		//num_ = -1;
	}

	int write(int deviceID, int address, int dat){

		RbWire[num_]->beginTransmission(deviceID);
		RbWire[num_]->write(address);
		RbWire[num_]->write(dat);
	    return RbWire[num_]->endTransmission();
	}

	int read(int n, int deviceID, int addrL, int addrH){
		int dat = 0;
		byte datH;

		RbWire[num_]->beginTransmission(deviceID);
		RbWire[num_]->write(addrL);
		RbWire[num_]->endTransmission();

		RbWire[num_]->requestFrom(deviceID, 1);
		dat = RbWire[num_]->read();

		if(n >= 3){
			RbWire[num_]->beginTransmission(deviceID);
			RbWire[num_]->write(addrH);
			RbWire[num_]->endTransmission();

			RbWire[num_]->requestFrom(deviceID, 1);
			datH = RbWire[num_]->read();

			dat += (datH<<8);
		}
		return dat;
	}

	void beginTransmission(int deviceID){
		RbWire[num_]->beginTransmission(deviceID);
	}

	int lwrite(int dat){
		return RbWire[num_]->write(dat);
	}

	int endTransmission(){
		return RbWire[num_]->endTransmission( 1 );
	}

	int endTransmission(uint8_t sendStop){
		return RbWire[num_]->endTransmission(sendStop);
	}

	int	requestFrom(int addr, int cnt){
		 return RbWire[num_]->requestFrom(addr, cnt);
	}

	int read(){
		return RbWire[num_]->read();
	}

	int available(){
		return RbWire[num_]->available();
	}

	void frequency(int freq){
		RbWire[num_]->setFrequency(freq);
	}

private:
	int num_;
};

//**************************************************
// メモリの開放時に走る
//**************************************************
static void i2c_free(mrb_state *mrb, void *ptr) {
	I2c2* i2c = static_cast<I2c2*>(ptr);
	delete i2c;
}

//**************************************************
// この構造体の意味はよくわかっていない
//**************************************************
static struct mrb_data_type i2c_type = { "I2c", i2c_free };

//**************************************************
// I2Cを初期化します: I2c.new
//  I2c.new(num)　
//  num: 通信番号(0:SDA-0/SCL-1, 1:SDA-5/SCL-6, 2:SDA-7/SCL-8, 3:SDA-12/SCL-11, 4:SDA-9(26)/SCL-3)
//
// 戻り値
//  I2cのインスタンス
//**************************************************
static mrb_value mrb_i2c_initialize(mrb_state *mrb, mrb_value self) {
	// Initialize data type first, otherwise segmentation fault occurs.
	DATA_TYPE(self) = &i2c_type;
	DATA_PTR(self) = NULL;

mrb_int num;

	mrb_get_args(mrb, "i", &num);

	if (num < 0 && num >= WIRE_MAX)
	{
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	I2c2* i2c = new I2c2(num);

	DATA_PTR(self) = i2c;
	return self;
}

//**************************************************
// アドレスにデータを書き込みます: I2c.write
//  I2c.write( deviceID, address, data )
//	deviceID: デバイスID
//  address: 書き込みアドレス
//  data: データ
//
//  戻り値は以下のとおり
//		0: 成功
//		4: その他のエラー
//**************************************************
mrb_value mrb_i2c_write(mrb_state *mrb, mrb_value self)
{
int deviceID, addr, dat;

	mrb_get_args(mrb, "iii", &deviceID, &addr, &dat);

	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));

    return mrb_fixnum_value(i2c->write(deviceID, addr, dat));
}

////**************************************************
// アドレスからデータを読み込み: I2c.read
//  I2c.read( deviceID, addressL[, addressH] )
//	deviceID: デバイスID
//  addressL: 読み込み下位アドレス
//  addressH: 読み込み上位アドレス
//
//  戻り値は読み込んだ値
//**************************************************
mrb_value mrb_i2c_read(mrb_state *mrb, mrb_value self)
{
int deviceID, addrL;
int addrH = 0;

	int n = mrb_get_args(mrb, "ii|i", &deviceID, &addrL, &addrH);

	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));

    return mrb_fixnum_value(i2c->read(n, deviceID, addrL, addrH));
}

//**************************************************
// I2Cデバイスに対して送信を開始するための準備をする: I2c.begin
//	I2c.begin( deviceID )
//	この関数は送信バッファを初期化するだけで、実際の動作は行わない。繰り返し呼ぶと、送信バッファが先頭に戻る。
//	deviceID: デバイスID 0～0x7Fまでの純粋なアドレス
//**************************************************
mrb_value mrb_i2c_beginTransmission(mrb_state *mrb, mrb_value self)
{
int deviceID;

	mrb_get_args(mrb, "i", &deviceID);

	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));

	i2c->beginTransmission(deviceID);

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// 送信バッファの末尾に数値を追加する: I2c.lwrite
//	I2c.lwrite( data )
//	data: セットする値
//
// 戻り値は、送信したバイト数(バッファに溜めたバイト数)を返す。
//	送信バッファ(260バイト)に空き容量が無ければ失敗して0を返す
//**************************************************
mrb_value mrb_i2c_lwrite(mrb_state *mrb, mrb_value self)
{
int dat;

	mrb_get_args(mrb, "i", &dat);

	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));

	return mrb_fixnum_value(i2c->lwrite(dat));
}

//**************************************************
// デバイスに対してI2Cの送信シーケンスを発行する: I2c.end
//	I2c.end([sendStop])
//	I2Cの送信はこの関数を実行して初めて実際に行われる。
//  stop: 0:ストップコンディション発生させない、1:ストップコンディション発生させる
//　　　　省略したときは、1:ストップコンディション発生させる
//
// 戻り値は以下のとおり
//	0: 成功
//	4: その他のエラー
//**************************************************
mrb_value mrb_i2c_endTransmission(mrb_state *mrb, mrb_value self)
{
int dat;

	int n = mrb_get_args(mrb, "|i", &dat);

	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));

	if(n == 0){
		return mrb_fixnum_value(i2c->endTransmission(1));
	}

	return mrb_fixnum_value(i2c->endTransmission(dat));
}

//**************************************************
// デバイスに対して受信シーケンスを発行しデータを読み出す: I2c.request
//	I2c.request( address, count )
//  num: 通信番号(0:SDA-0/SCL-1, 1:SDA-5/SCL-6, 2:SDA-7/SCL-8, 3:SDA-12/SCL-11, 4:SDA-9(26)/SCL-3)
//	address: 読み込み開始アドレス
//	count: 読み出す数
//
//  戻り値は、実際に受信したバイト数。
//**************************************************
mrb_value mrb_i2c_requestFrom(mrb_state *mrb, mrb_value self)
{
int addr, cnt;

	mrb_get_args(mrb, "ii", &addr, &cnt);

	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));

    return mrb_fixnum_value(i2c->requestFrom(addr, cnt));
}

//**************************************************
// デバイスに対して受信シーケンスを発行しデータを読み出す: I2c.lread
//	I2c.lread()
//
//  戻り値は読み込んだ値
//**************************************************
mrb_value mrb_i2c_lread(mrb_state *mrb, mrb_value self)
{
	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));
	return mrb_fixnum_value(i2c->read());
}

//**************************************************
// デバイスに対して受信バッファ内にあるデータ数を調べる: I2c.available
//	I2c.available()
//
//  戻り値はデータ数
//**************************************************
mrb_value mrb_i2c_available(mrb_state *mrb, mrb_value self)
{
	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));
	return mrb_fixnum_value(i2c->available());
}

//**************************************************
// 周波数を変更する: I2c.frequency
//  I2c.frequency( Hz )
//  Hz: クロックの周波数をHz単位で指定する。
//**************************************************
mrb_value mrb_i2c_frequency(mrb_state *mrb, mrb_value self)
{
int freq;

	mrb_get_args(mrb, "i", &freq);

	I2c2* i2c = static_cast<I2c2*>(mrb_get_datatype(mrb, self, &i2c_type));

	i2c->beginTransmission(freq);

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// ライブラリを定義します
//**************************************************
void i2c_Init(mrb_state *mrb)
{
	//0:Wire, 1:Wire1, 2:Wire2, 3:Wire3 4:Wire4 5:Wire5
	RbWire[0] = &Wire;
	RbWire[1] = &Wire1;
	RbWire[2] = &Wire2;
	RbWire[3] = &Wire3;
	RbWire[4] = &Wire4;
	RbWire[5] = &Wire5;

	struct RClass *i2cModule = mrb_define_class(mrb, "I2c", mrb->object_class);
	MRB_SET_INSTANCE_TT(i2cModule, MRB_TT_DATA);

	mrb_define_method(mrb, i2cModule, "initialize", mrb_i2c_initialize, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, i2cModule, "write", mrb_i2c_write, MRB_ARGS_REQ(3));
	mrb_define_method(mrb, i2cModule, "read", mrb_i2c_read, MRB_ARGS_REQ(2)|MRB_ARGS_OPT(1));
	mrb_define_method(mrb, i2cModule, "begin", mrb_i2c_beginTransmission, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, i2cModule, "lwrite", mrb_i2c_lwrite, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, i2cModule, "end", mrb_i2c_endTransmission, MRB_ARGS_OPT(1));
	mrb_define_method(mrb, i2cModule, "request", mrb_i2c_requestFrom, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, i2cModule, "lread", mrb_i2c_lread, MRB_ARGS_NONE());
	mrb_define_method(mrb, i2cModule, "available", mrb_i2c_available, MRB_ARGS_NONE());
	mrb_define_method(mrb, i2cModule, "frequency", mrb_i2c_frequency, MRB_ARGS_REQ(1));
}
