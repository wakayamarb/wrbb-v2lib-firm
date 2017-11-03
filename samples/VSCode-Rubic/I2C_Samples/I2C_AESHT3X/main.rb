#スレーブのアドレス
I2C_ADDR = 0x45
#データシートの設定条件より
COMMAND = 0x2400


usb = Serial.new(0)
#引数は初期化する通信ポート(SDA/SCL)
wire = I2c.new(0)

10.times do
  #書き込み準備の開始
  wire.begin(I2C_ADDR);
  #コマンドMSB部
  wire.lwrite(COMMAND>>8);
  #コマンドLSB部 0xFFにてマスクする。0xFF.to_s(2) => 11111111
  wire.lwrite(COMMAND&0xFF);
  #I2C通信。wire.endでスレーブに送信する
  wire.end

  #スレーブからデータを受信する
  wire.request(I2C_ADDR,6)
  data = []
  num = 0
  while wire.available != 0 do
    data[num] = wire.lread
    num += 1
  end

  #データシートの式から温度と湿度を計算する
  temp = -45.0 + (175.0 * ((data[0] * 256.0) + data[1]) / 65535.0)
  humidity = (100.0 * ((data[3] * 256.0) + data[4]) / 65535.0)
 
  
  #コンソールに出力する
  usb.println("T : #{temp.to_s}")
  usb.println("RH : #{humidity.to_s}")
  usb.println("+++++++++++++")
  delay(1000)
end