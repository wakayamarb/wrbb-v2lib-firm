#!mruby
#################################
# I2C LPS331AP
#################################
#ここには全体で使用する変数を定義しています
@Interval = 500				# 間隔処理の間隔時間が入る(ms)
@Usb = Serial.new(0, 115200)		#USBシリアル通信の初期化
@Dev = I2c.new(0)
@LPS331AP = 0x5D			# 0b01011101 圧力・温度センサのアドレス
delay(300)	#300ms待つ

###################################
# 気圧と温度センサの初期化
###################################
def initTempPress()
	#LPS331AP_Write    0xBA    //0b10111010
	#LPS331AP_WHO_AM_I = 0x0F
	LPS331AP_CTRL_REG1 = 0x20	# Control register
	LPS331AP_SAMPLING = 0xA0	# A0:7Hz, 90:1Hz

	#val = @Dev.read( LPS331AP, APTemp_WHO_AM_I )
	#@Usb.println(0, "LPS331AP Who am I? = " + val.to_s)

	# 7Hz
	@Dev.write(@LPS331AP, LPS331AP_CTRL_REG1, LPS331AP_SAMPLING)
	delay(100)	#100ms待つ
end

###################################
# I2CのREAD1
###################################
def I2cRead1(id, addL)
    @Dev.begin(id)
    @Dev.lwrite(addL)
    @Dev.end
    @Dev.request(id, 1)
    dl = @Dev.lread()
    return dl
end

###################################
# I2CのREAD2
###################################
def I2cRead2(id, addL, addH)
    @Dev.begin(id)
    @Dev.lwrite(addL)
    @Dev.end
    @Dev.request(id, 1)
    dl = @Dev.lread()
	@Dev.begin(id)
    @Dev.lwrite(addH)
    @Dev.end
	@Dev.request(id, 1)
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
@Usb.println("LPS331AP System Start.")

# 気圧と温度センサの初期化
initTempPress()

#次の動作時間をセットする
toTime = millis() + @Interval
	
sw = 1
#無限にループします
#while(true) do
20.times do    
		
	#間隔待ち。予定の時間になるまで待っている
	while(millis() < toTime)do
		delay(1)	#1ms待ちます
	end

	#次の設定時間を計算します
	toTime = nextTime(toTime)
	@Usb.print(toTime.to_s + ",")

	#気圧を取得します --------------------------------------
	#Address 0x28, 0x29, 0x2A, 0x2B, 0x2C
	v0 = @Dev.read( @LPS331AP, 0x28, 0x29)
	v1 = @Dev.read( @LPS331AP, 0x2A)
	a = v0 + v1 * 65536
	a = a / 4096.0		# hPa単位に直す
	@Usb.print(a.to_s + ",")
	#---------------------------------------------------------
		
	#温度を取得します --------------------------------------
	v2 = @Dev.read( @LPS331AP, 0x2B, 0x2C)
	if v2 > 32767
		v2 = v2 - 65536
	end
	t = v2 / 480.0 + 42.5
	@Usb.print(t.to_s + ",")
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
