/*
 * シリアル通信関連
 *
 * Copyright (c) 2016 Wakayama.rb Ruby Board developers
 *
 * This software is released under the MIT License.
 * https://github.com/wakayamarb/wrbb-v2lib-firm/blob/master/MITL
 *
 */
#include <Arduino.h>

#include <mruby.h>
#include <mruby/array.h>
#include <mruby/data.h>
#include <mruby/class.h>
#include <mruby/string.h>

#include "../wrbb.h"

#define SERIAL_MAX	6

HardwareSerial *RbSerial[SERIAL_MAX];		//0:Serial(USB), 1:Serial1, 2:Serial3, 3:Serial2, 4:Serial6 5:Serial7

unsigned char BuffRead[SERIAL_BUFFER_SIZE];

//**************************************************
// SerialをラップしたSerialcクラス作成
//**************************************************
class Serialc {
public:
	Serialc(int num){
		num_ = num;
	}
	virtual ~Serialc() {
		//num_ = -1;
	}

	void baudrate(int bps){
		RbSerial[num_]->begin(bps);
	}

	void println(){
		RbSerial[num_]->println();
	}

	void print(char *str){
		RbSerial[num_]->print(str);
	}

	void println(char *str){
		RbSerial[num_]->println(str);
	}

	int available(){
		return RbSerial[num_]->available();
	}

	int read(){
		int len = RbSerial[num_]->available();

		for(int i=0; i<len; i++){
			BuffRead[i] = RbSerial[num_]->read();
		}
		return len;
	}

	int write(const unsigned char *str, int len){
		return RbSerial[num_]->write(str, len);
	}

	void flush(){
		RbSerial[num_]->flush();
	}

private:
	int num_;
};


//**************************************************
// メモリの開放時に走る
//**************************************************
static void serial_free(mrb_state *mrb, void *ptr) {
	Serialc* serial = static_cast<Serialc*>(ptr);
	delete serial;
}

//**************************************************
// この構造体の意味はよくわかっていない
//**************************************************
static struct mrb_data_type serial_type = { "Serial", serial_free };


//**************************************************
// シリアル通信を初期化します: Serial.new
//  Serial.new(num[, bps])　
//  num: 通信番号(0:USB, 1:TX-0/RX-1, 2:TX-5/RX-6, 3:TX-7/RX-8, 4:TX-12/RX-11, 5:TX-9(26)/RX-3)
//  bps: ボーレート
//
// 戻り値
//  Serialのインスタンス
//**************************************************
static mrb_value mrb_serial_initialize(mrb_state *mrb, mrb_value self) {
	// Initialize data type first, otherwise segmentation fault occurs.
	DATA_TYPE(self) = &serial_type;
	DATA_PTR(self) = NULL;

mrb_int num;
mrb_int bps;

	int n =  mrb_get_args(mrb, "i|i", &num,&bps);

	if (num < 0 && num >= SERIAL_MAX)
	{
		return mrb_nil_value();			//戻り値は無しですよ。
	}

	Serialc* serialc = new Serialc(num);

	if(n >= 2){
		serialc->baudrate(bps);
	}
 

	DATA_PTR(self) = serialc;
	return self;
}

//**************************************************
// ボーレートを設定します: Serial.bps
//  Serial.bps(bps)
//  bps: ボーレート 
//**************************************************
mrb_value mrb_serial_bps(mrb_state *mrb, mrb_value self)
{
int bps;

	Serialc* serialc = static_cast<Serialc*>(mrb_get_datatype(mrb, self, &serial_type));

	mrb_get_args(mrb, "i", &bps);

	serialc->baudrate(bps);

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// シリアルに出力します: Serial.print
//  Serial.print([str])
//  str: 文字列
//    省略時は何も出力しません
//**************************************************
mrb_value mrb_serial_print(mrb_state *mrb, mrb_value self)
{
mrb_value text;
	
	Serialc* serialc = static_cast<Serialc*>(mrb_get_datatype(mrb, self, &serial_type));

	int n = mrb_get_args(mrb, "|S", &text);

	if(n > 0){
		serialc->write((const unsigned char *)RSTRING_PTR(text), RSTRING_LEN(text));
	}
	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// シリアルに\r\n付きで出力します: Serial.println
//  Serial.println([str])
//  str: 文字列
//    省略時は改行のみ
//**************************************************
mrb_value mrb_serial_println(mrb_state *mrb, mrb_value self)
{
mrb_value text;
	
	Serialc* serialc = static_cast<Serialc*>(mrb_get_datatype(mrb, self, &serial_type));

	int n = mrb_get_args(mrb, "|S", &text);

	if(n > 0){
		serialc->write((const unsigned char *)RSTRING_PTR(text), RSTRING_LEN(text));
	}
	serialc->println();

	return mrb_nil_value();			//戻り値は無しですよ。
}

//**************************************************
// シリアルデータがあるかどうか調べます: Serial.available
//  Serial.available()
//  戻り値 シリアルバッファにあるデータのバイト数。0の場合はデータなし
//**************************************************
mrb_value mrb_serial_available(mrb_state *mrb, mrb_value self)
{
	Serialc* serialc = static_cast<Serialc*>(mrb_get_datatype(mrb, self, &serial_type));
	
	return mrb_fixnum_value(serialc->available());
}

//**************************************************
// シリアルからデータを取得します: Serial.read
//  Serial.read()
// 戻り値
//	データ配列
//**************************************************
mrb_value mrb_serial_read(mrb_state *mrb, mrb_value self)
{
	Serialc* serialc = static_cast<Serialc*>(mrb_get_datatype(mrb, self, &serial_type));

	int len = serialc->read();
	return mrb_str_new(mrb, (const char*)BuffRead, len);
}

//**************************************************
// シリアルにデータを出力します: Serial.write
//  Serial.write(buf,len)
//	buf: 出力データ
//	len: 出力データサイズ
// 戻り値
//	出力したバイト数
//**************************************************
mrb_value mrb_serial_write(mrb_state *mrb, mrb_value self)
{
mrb_value value;
int len;

	Serialc* serialc = static_cast<Serialc*>(mrb_get_datatype(mrb, self, &serial_type));

	mrb_get_args(mrb, "Si", &value, &len);

	return mrb_fixnum_value( serialc->write( (const unsigned char *)RSTRING_PTR(value), len));
}

//**************************************************
// シリアルデータをフラッシュします: Serial.flash
//  Serial.flash()
//**************************************************
mrb_value mrb_serial_flash(mrb_state *mrb, mrb_value self)
{
	Serialc* serialc = static_cast<Serialc*>(mrb_get_datatype(mrb, self, &serial_type));

	serialc->flush();

	return mrb_nil_value();			//戻り値は無しですよ。
}

//unsigned char WiFiData[256];
////**************************************************
//// USBポートとESP8266をシリアルで直結します: WiFi.bypass
////  WiFi.bypass()
//// リセットするまで、処理は戻りません。
////**************************************************
//mrb_value mrb_wifi_bypass(mrb_state *mrb, mrb_value self)
//{
//	int len0, len1,len;
//
//	//RbSerial[0]->begin(115200);
//	//RbSerial[3]->begin(115200);
//
//	int retCnt = 0;
//	while(true){
//		len0 = RbSerial[0]->available();
//		len1 = RbSerial[3]->available();
//
//		if(len0 > 0){
//			len = len0<256 ? len0 : 256;
//
//			for(int i=0; i<len; i++){
//				WiFiData[i] = (unsigned char)RbSerial[0]->read();
//
//				if(WiFiData[i] == 0x0d){ //0x0Dのみの改行を連打したらbypassモードを抜ける
//					retCnt++;
//					if(retCnt > 20){
//						return mrb_nil_value();			//戻り値は無しですよ。
//					}
//				}
//				else{
//					retCnt = 0;
//				}
//			}
//			RbSerial[3]->write( WiFiData, len );
//		}
//
//		if(len1 > 0){
//			len = len1<256 ? len1 : 256;
//			
//			for(int i=0; i<len; i++){
//				WiFiData[i] = (unsigned char)RbSerial[3]->read();
//			}
//	        RbSerial[0]->write( WiFiData, len );
//		}
//	}
//	return mrb_nil_value();			//戻り値は無しですよ。
//}


//**************************************************
// ライブラリを定義します
//**************************************************
void serial_Init(mrb_state* mrb) {

	RbSerial[0] = &Serial;	//0:Serial(USB)
	RbSerial[1] = &Serial1;	//1:Serial1
	RbSerial[2] = &Serial2;	//2:Serial3
	RbSerial[3] = &Serial3;	//3:Serial2
	RbSerial[4] = &Serial4;	//4:Serial6
	RbSerial[5] = &Serial5;	//5:Serial7

	struct RClass *serialModule = mrb_define_class(mrb, "Serial", mrb->object_class);
	MRB_SET_INSTANCE_TT(serialModule, MRB_TT_DATA);

	mrb_define_method(mrb, serialModule, "initialize", mrb_serial_initialize, MRB_ARGS_REQ(1)|MRB_ARGS_OPT(1));

	mrb_define_method(mrb, serialModule, "bps", mrb_serial_bps, MRB_ARGS_REQ(1));
	mrb_define_method(mrb, serialModule, "print", mrb_serial_print, MRB_ARGS_OPT(1));
	mrb_define_method(mrb, serialModule, "println", mrb_serial_println, MRB_ARGS_OPT(1));
	mrb_define_method(mrb, serialModule, "read", mrb_serial_read, MRB_ARGS_NONE());
	mrb_define_method(mrb, serialModule, "write", mrb_serial_write, MRB_ARGS_REQ(2));
	mrb_define_method(mrb, serialModule, "flash", mrb_serial_flash, MRB_ARGS_NONE());
	mrb_define_method(mrb, serialModule, "available", mrb_serial_available, MRB_ARGS_NONE());
	
	//struct RClass *wifiModule = mrb_define_module(mrb, "WiFi");
	//mrb_define_module_function(mrb, wifiModule, "bypass", mrb_wifi_bypass, MRB_ARGS_NONE());
}
