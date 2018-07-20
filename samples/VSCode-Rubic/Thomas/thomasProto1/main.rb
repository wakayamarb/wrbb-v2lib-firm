#!mruby
#Ver2.50
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
ML = 0    #左のモーター
MR = 1    #右のモーター
Vero = [0,0]
Vpin = [4,10]
Cpin = [[0,3],[15,14]]
MaxErrCnt = 10
ErrCnt = 0
MaxDatInCnt = 1000
DatInCnt = 0
WiFiEN = 5          #WiFiのEN:LOWでDisableです
VL53Enable = true   #VL53L0Xを使うかどうか
IsConnectVL53 = false #VL53L0Xが接続されているかどうか
Distance = 80   #mm以下まで近づくと止まる
LCD_ADD = 0x3E
BlnTxt = false    #テキストを受信するフラグ

#Usb = Serial.new(0,9600)
Usb = Serial.new(0,115200)

pinMode(Cpin[ML][0], OUTPUT)  #出力設定
pinMode(Cpin[ML][1], OUTPUT)  #出力設定
pinMode(Cpin[MR][0], OUTPUT)  #出力設定
pinMode(Cpin[MR][1], OUTPUT)  #出力設定
pinMode(WiFiEN, OUTPUT) #WiFi_ENのピン設定

#ESP8266を一度停止させる(リセットと同じ)
digitalWrite(WiFiEN,LOW)   # LOW:Disable
#delay 500

#I2Cの設定
LCD = I2c.new(0)
if(System.use?("VL53L0X"))
  IsConnectVL53 = true
  VL53L0X.setI2C(0)     	#I2Cの0番に接続します
  #VL53L0X.begin
	VL53L0X.init
	VL53L0X.startContinuous
end

#-------
#液晶へ１コマンド出力
#-------
def lcd_cmd(cmd)
	LCD.write(LCD_ADD,0x00,cmd)
	if((cmd == 0x01)||(cmd == 0x02))then
			delay 2
	else
			delay 0
	end
end
#-------
#データを送る
#-------
def lcd_data(dat)
	LCD.write(LCD_ADD,0x40,dat)
	delay 0
end
#-------
#カーソルのセット
#-------
def lcd_setCursor(clm,row)
	if(row == 0)
			lcd_cmd(0x80 + clm)
	else
			lcd_cmd(0xc0 + clm)
	end
end
#-------
#LCDの初期化
#-------
def lcd_begin()
	#puts "lcd_begin"
	#digitalWrite(RST, LOW)
	#delay(1)
	#digitalWrite(RST, HIGH)
	#delay(40)
	lcd_cmd(0x38)   #8bit 2line Normal mode
	lcd_cmd(0x39)   #8bit 2line Extend mode
	lcd_cmd(0x14)   #OSC 183Hz BIAS 1/5

	#コントラスト設定
	contrast = 0x5F
	lcd_cmd(0x70 + (contrast & 0x0F))   		#下位4bit
	lcd_cmd(0x5C + ((contrast >> 4)& 0x3))  #上位2bit
	#lcd_cmd(0x6B)                   				#Follwer for 3.3V
	lcd_cmd(0x6C)                   				#Follwer for 3.3V
	delay(300)

	lcd_cmd(0x38)       #Set Normal mode
	lcd_cmd(0x0C)       #Display On
	lcd_cmd(0x01)       #Clear Display
	delay(2)
	#puts "end of lcd_begin"
end
#LCDの初期化
lcd_begin()
#-------
#全消去関数
#-------
def lcd_clear()
	lcd_cmd(0x01)   	#初期化コマンド出力
	delay(2)
end
#-------
#文字列表示関数
#-------
def lcd_print(cs)
	cs.each_byte{|c| lcd_data(c)}
end
#-------
# 前に障害物があるかどうか調べます
#-------
def sensorChk(p, tp)
  if(VL53Enable && IsConnectVL53)
    len = VL53L0X.readContinuous
    lcd_setCursor(0, 0)
    lcd_print(len.to_s + " ")
    if(len < Distance)
      if(p > 0 && tp > 0)
        #puts len
        mMove(ML, 0)
        mMove(MR, 0)
        true
        return
      elsif(p == 0 && tp == 0)
        #puts len
        if(digitalRead(Cpin[MR][0]) == LOW \
          && digitalRead(Cpin[MR][1]) == HIGH \
          && digitalRead(Cpin[ML][0]) == LOW \
          && digitalRead(Cpin[ML][1]) == HIGH)
          puts len
          mMove(ML, 0)
          mMove(MR, 0)
          true
          return
        end
      end
    end
  end
  false
end
#-------
# p==0 -> 停止に
# p>0 -> CW , p<0 -> CCW に設定します
#-------
def mMove(dir,p)
  if(p == 0)
    digitalWrite(Cpin[dir][0],LOW)
    digitalWrite(Cpin[dir][1],LOW)
    Vero[dir] = 0
  elsif(p < 0)
    digitalWrite(Cpin[dir][0], HIGH)
    digitalWrite(Cpin[dir][1], LOW)
    Vero[dir] = -p
  else
    digitalWrite(Cpin[dir][0], LOW)
    digitalWrite(Cpin[dir][1], HIGH)
    Vero[dir] = p
  end
  pwm(Vpin[dir], Vero[dir])
end
#-------
# HEX 6バイトデータの読み込み
# 先頭の2文字が X のデータ(0x00～0xFF)
# 次の2文字が Yのデータ(0x00～0xFF)
# 次の2文字が ボタンのビット(0x00～0xFF)
# Xはプロポの前後を示す。127が中央0を意味する。127～255が前進、0～127が後退
# Yは左右を示す。127が中央0を意味する。127～255が右へ、0～127が左。
# 傾きが大きくなるほど、急に曲がる。最大にすると、超信地旋回になる。
#
# センサ感度 dt=10 (値は感度範囲距離)
# センサ許可 se=0 (0:無効, 1:有効)
#-------
def command(cmd)
  puts cmd
  x = cmd[0..1].hex
  p = 0
  if(x == 0)
    p = -255
  else
    p = ((cmd[0..1].hex - 127) * 1.9922).round
  end

  y = cmd[2..3].hex

  turn = 0
  tp = p
  if(y == 127)
    #前進 or 後退
    if(!sensorChk(p,p))
      mMove(ML, p)
      mMove(MR, p)
    end      
  else
    turn = (y - 127.0) / 128.0

    if(turn < 0)
      #MLを変化させる
      turn = -1 if(turn < -0.99) #turnが-0.99以下なら -1とする
      tp = (p * (turn + 0.5) / 0.5).round

      if(!sensorChk(p,tp))
        mMove(ML, tp)
        mMove(MR, p)
      end
    else
      #MRを変化させる
      tp = (-p * (turn - 0.5) / 0.5).round
      if(!sensorChk(p,tp))
        mMove(ML, p)
        mMove(MR, tp)
      end      
    end
  end

  btn = cmd[4..5].hex
  if(btn >= 0x80)
    BlnTxt = true
  end

  if( ((btn >> 2) & 1) == 1)
    if(VL53Enable)
      VL53Enable = false
    else
      VL53Enable = true
    end
  end

  puts p.to_s + ", " + turn.to_s + ", " + tp.to_s
end
#-------
#テキストデータを受信してLCDに表示します
#-------
def recvText(cmd)
  while(Usb.available > 0)do
    c = Usb.read
    for i in 0..(c.length - 1)
      cmd += c[i]
      if(c[i] == "\r")
        lcd_clear()             #全消去
        if(cmd.length <= 8)
          lcd_setCursor(0, 0)
          lcd_print(cmd[0..cmd.length - 1])
        else
          lcd_setCursor(0, 0)
          lcd_print(cmd[0..7])
          lcd_setCursor(0, 1)
          lcd_print(cmd[8..cmd.length-1])
        end
        BlnTxt = false
        cmd = ""  
      end
    end
  end
  cmd
end
#-------
#受信データを"XXYYBB\r"に切り分けて処理します
#-------
def recvData(cmd)
  while(Usb.available > 0)do
    c = Usb.read
    lcd_setCursor(0, 0)
    lcd_print(c)

    for i in 0..(c.length - 1)
      cmd += c[i]
      Usb.print c[i]
      if(c[i] == "\r")

        if(cmd.length == 7)
          #コマンド処理します
          command(cmd)
          ErrCnt = 0
          if(BlnTxt)
            #cmd=""
            return ""
          end
        else
          ErrCnt += 1
        end
      
        if(ErrCnt > MaxErrCnt)
          puts cmd.length
          puts ErrCnt
          mMove(ML, 0)
          mMove(MR, 0)
          ErrCnt = 0
        end
        cmd = ""
      end
    end
  end
  cmd
end
#-----------------------------------------

lcd_clear()             #全消去
lcd_setCursor(0, 0)
lcd_print("Thomas  ")
lcd_setCursor(0, 1)
lcd_print("Starting")
#Usb.println("Thomas System Starting")

cmd = ""
lcd_setCursor(0, 0)
lcd_print("        ")
while(true)do
  if(BlnTxt)
    cmd = recvText(cmd) 
  else
    cmd = recvData(cmd)
  end

  if(cmd.length > 0)
    DatInCnt += 1
    if(DatInCnt > MaxDatInCnt)
      mMove(ML, 0)
      mMove(MR, 0)
      DatInCnt = 0
      cmd = ""
    end
  else
    DatInCnt = 0
  end
  
  sensorChk(0,0)

  delay 0
end