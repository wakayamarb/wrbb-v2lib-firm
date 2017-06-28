#!mruby
#################################
# I2C KXSD9-2050
#################################
#ここには全体で使用する変数を定義しています
@KXSD9_2050 = 0x19				# 0b0011000 加速度センサのアドレス
@KXSD9_2050_vpg = 273			# 00:205, 01:273, 02:410, 03:819 加速度センサの分解能(Value/G)
@KXSD9_2050_zero = 2048			# 0Gの時の値
@Interval = 500				    # 間隔処理の間隔時間が入る(ms)
@Usb = Serial.new(0, 115200)	#USBシリアル通信の初期化
@Dev = I2c.new(0)
delay(300)	#300ms待つ
#@Dev.frequency(500000)

###################################
# I2CのWrite
###################################
def I2cWrite(id, add, dat)
    @Dev.begin(id)
    @Dev.lwrite(add)
    @Dev.lwrite(dat)
    @Dev.end(1)
end

###################################
# 加速度センサの初期化
###################################
def initAccel()
	
	# Accel Setup [KXSD9-2050]
 	KXSD9_2050_CTRL_REGC = 0x0C
	KXSD9_2050_SENS = 0x01  		# 00:+-8G, 01:+-6G, 02:+-4G, 03: +-2G
	#@KXSD9_2050_vpg = 273			# 00:205, 01:273, 02:410, 03:819
#	KXSD9_2050_FILLTER = 0xC0		# 00:Non, 20:2000Hz, 80:1000Hz, A0:500Hz, C0:100Hz, E0:50Hz
	KXSD9_2050_FILLTER = 0xC0		# 00:Non, 20:2000Hz, 80:1000Hz, A0:500Hz, C0:100Hz, E0:50Hz
 
	# SENSとFILLTERを設定
	@Dev.write(@KXSD9_2050, KXSD9_2050_CTRL_REGC, KXSD9_2050_SENS | KXSD9_2050_FILLTER)
	#I2cWrite(@KXSD9_2050, KXSD9_2050_CTRL_REGC, KXSD9_2050_SENS | KXSD9_2050_FILLTER)

	delay(100)	#100ms待つ
end

###################################
# I2CのREAD1
###################################
def I2cRead1(id, addL)
    @Dev.begin(id)
    @Dev.lwrite(addL)
    @Dev.end(0)
    @Dev.request(id, 1)
    dl = @Dev.lread()
    return dl
end

###################################
# I2CのREAD2
###################################
def I2cRead2(id, addL, addH)
    @Dev.begin(id)
    delay 1
    @Dev.lwrite(addL)
    delay 1
    @Dev.end(0)
    delay 1
    @Dev.request(id, 1)
    delay 1
    dl = @Dev.lread()
    delay 1
	@Dev.begin(id)
    delay 1
    @Dev.lwrite(addH)
    delay 1
    @Dev.end(0)
    delay 1
	@Dev.request(id, 1)
    delay 1
    dh = @Dev.lread()
    return dh*256 + dl
end

###################################
# 次の設定時間を計算します
###################################
def nextTime(toTime)
	toTime = toTime + @Interval
	#次の設定時間を計算します
	while(millis() > toTime)do
		toTime = toTime + @Interval
		delay(1)	#1ms待ちます
	end
	return toTime
end


################ ここからMainプログラム #################
@Usb.println("System Start.")

# 加速度センサの初期化
initAccel()

#次の動作時間をセットする
toTime = millis() + @Interval
	
sw = 1
#無限にループします
#while(true) do
15.times do
		
	#間隔待ち。予定の時間になるまで待っている
	while(millis() < toTime)do
		delay(1)	#1ms待ちます
	end

	#次の設定時間を計算します
	toTime = nextTime(toTime)
	@Usb.print(toTime.to_s + ",")

	#加速度を取得します --------------------------------------
	#Address 0x00, 0x01, 0x02, 0x03, 0x04, 0x05
	v0 = @Dev.read(@KXSD9_2050, 1, 0)
	v1 = @Dev.read(@KXSD9_2050, 3, 2)
	v2 = @Dev.read(@KXSD9_2050, 5, 4)
	#v0 = I2cRead2(@KXSD9_2050, 1, 0)
	#v1 = I2cRead2(@KXSD9_2050, 3, 2)
	#v2 = I2cRead2(@KXSD9_2050, 5, 4)
	x = ((v0 / 16).truncate - @KXSD9_2050_zero) / @KXSD9_2050_vpg
	y = ((v1 / 16).truncate - @KXSD9_2050_zero) / @KXSD9_2050_vpg
	z = ((v2 / 16).truncate - @KXSD9_2050_zero) / @KXSD9_2050_vpg
	@Usb.print(x.to_s + "," + y.to_s + "," + z.to_s)
	#---------------------------------------------------------
	
	@Usb.println(",")
		
	#LEDを点滅させます
	led(sw)
	sw = 1 - sw

    #if(digitalRead(10) != 0)then
    #    break
    #end
end
#終わり
