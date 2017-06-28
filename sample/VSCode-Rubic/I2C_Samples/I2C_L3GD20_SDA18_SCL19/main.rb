#!mruby
#################################
# I2C L3GD20
#################################
#ここには全体で使用する変数を定義しています
@L3GD20 = 0x6A				    # 0b01101011 ジャイロセンサのアドレス ADDRESS-> 0x6A or 0x6B
@L3GD20_dps = 0.0175	        # 00:0.00875, 10:0.0175, 30:0.07 ジャイロセンサの分解能(deg/s)
@Interval = 500				    # 間隔処理の間隔時間が入る(ms)
@Usb = Serial.new(0, 115200)	#USBシリアル通信の初期化
@Dev = I2c.new(0)
delay(300)	#300ms待つ

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
# ジャイロセンサの初期化
###################################
def initGyro()

	# Gyro Setup [L3GD20]
	#@L3GD20 = 0x6B				# 0b01101011
	#@L3GD20_Write = 0xD4				# 0b11010110
	#@L3GD20_WHO_AM_I = 0x0F
	L3GD20_CTRL_REG1 = 0x20		# Control register
	L3GD20_ENABLE = 0x0F
	L3GD20_CTRL_REG4 = 0x23		# Control register
	L3GD20_CTRL_REG3 = 0x22		# Control register
	L3GD20_RANGE = 0x10			# 00:+-250dps, 10:+-500dps, 30:+-2000dps
	#@L3GD20_dps = 0.0175		# 00:0.00875, 10:0.0175, 30:0.07

	#val = @Dev.read( L3GD20, L3GD20_WHO_AM_I)
	#@Usb.println("L3GD20 Who am I? = " + val.to_s)

	# Turn on all axes, disable power down
	@Dev.write(@L3GD20, L3GD20_CTRL_REG1, L3GD20_ENABLE)
	#I2cWrite(@L3GD20, L3GD20_CTRL_REG1, L3GD20_ENABLE)
	delay(100)	#100ms待つ
	# +-500dps
	@Dev.write(@L3GD20, L3GD20_CTRL_REG4, L3GD20_RANGE)
	#I2cWrite(@L3GD20, L3GD20_CTRL_REG4, L3GD20_RANGE)
	delay(100)	#100ms待つ
end

###################################
# I2CのREAD1
###################################
def I2cRead1(id, addL)
    @Dev.begin(id)
    @Dev.lwrite(addL)
    @Dev.end(1)
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
    @Dev.end(1)
    @Dev.request(id, 1)
    dl = @Dev.lread()
	@Usb.print(", " + dl.to_s)
	@Dev.begin(id)
    @Dev.lwrite(addH)
    @Dev.end(1)
	@Dev.request(id, 1)
    dh = @Dev.lread()
	@Usb.print(", " + dh.to_s)
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

# ジャイロセンサの初期化
initGyro()

#次の動作時間をセットする
toTime = millis() + @Interval
	
sw = 1
#無限にループします
while(true) do
		
	#間隔待ち。予定の時間になるまで待っている
	while(millis() < toTime)do
		delay(1)	#1ms待ちます
	end

	#次の設定時間を計算します
	toTime = nextTime(toTime)
	@Usb.print(toTime.to_s)

	#角速度を取得します --------------------------------------
	#Address 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D
	#v0 = @Dev.read(@L3GD20, 0x28, 0x29)
	#v1 = @Dev.read(@L3GD20, 0x2A, 0x2B)
	#v2 = @Dev.read(@L3GD20, 0x2C, 0x2D)
	v0 = I2cRead2(@L3GD20, 0x28, 0x29)
	v1 = I2cRead2(@L3GD20, 0x2A, 0x2B)
	v2 = I2cRead2(@L3GD20, 0x2C, 0x2D)
	if v0 > 32767
		v0 = v0 - 65536
	end
	if v1 > 32767
		v1 = v1 - 65536
	end
	if v2 > 32767
		v2 = v2 - 65536
	end
		
	v0 = v0 * @L3GD20_dps
	v1 = v1 * @L3GD20_dps
	v2 = v2 * @L3GD20_dps
	@Usb.print("," + v0.to_s + "," + v1.to_s + "," + v2.to_s )
	#---------------------------------------------------------

	@Usb.println(",")
		
	#LEDを点滅させます
	led(sw)
	sw = 1 - sw

    if(digitalRead(10) != 0)then
        break
    end
end
#終わり
