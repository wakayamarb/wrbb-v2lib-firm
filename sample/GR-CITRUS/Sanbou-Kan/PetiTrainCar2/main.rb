#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 90       #モータ速度速度の最大値を指定しています。0～255
MoveTime = 1000      #500msWaitさせたかったので用意している。
Vero = [4,10]       #モータの速度を決定するGR-CITRUSのピンが4番と10番です。     
Num = [18,3,15,14]  #モータの回転方向などを制御するビット、1モータ2ビットです。18,3番、15と14番がペアです
Sens = 17            #アナログ距離センサ
ID = 0x3E
Lcd = I2c.new(1)
RST = 6

Usb = Serial.new(0)
for i in Num do
 pinMode(i,OUTPUT)
end
pinMode(RST,OUTPUT)

#-------
# 液晶へ１コマンド出力
#-------
def lcd_cmd(cmd)
    Lcd.write(ID,0x00,cmd)

    if((cmd == 0x01)||(cmd == 0x02))then
        delay(2)
    else
        delay 0
    end
end

#-------
# 液晶へデータを送る
#-------
def lcd_data(dat)
    Lcd.write(ID,0x40,dat)
    delay 0
end

#-------
# 液晶のカーソ位置設定
#-------
def lcd_setCursor(clm,row)
    if(row==0)then
        lcd_cmd(0x80+clm)
    end
    
    if(row==1)then
        lcd_cmd(0xc0+clm)
    end
end

#-------
# 液晶の初期化
#-------
def lcd_begin()
    Usb.println("lcd_begin")
    digitalWrite(RST, LOW)
    delay(1)
    digitalWrite(RST, HIGH)
    delay(40)
    lcd_cmd(0x38)   #// 8bit 2line Normal mode
    lcd_cmd(0x39)   #// 8bit 2line Extend mode
    lcd_cmd(0x14)   #// OSC 183Hz BIAS 1/5

    #/* コントラスト設定 */
    contrast = 0x5F
    lcd_cmd(0x70 + (contrast & 0x0F))   #//下位4bit
    lcd_cmd(0x5C + ((contrast >> 4)& 0x3))     #//上位2bit
    #lcd_cmd(0x6B)                   #// Follwer for 3.3V
    lcd_cmd(0x6C)                   #// Follwer for 3.3V
    delay(300)

    lcd_cmd(0x38)       #// Set Normal mode
    lcd_cmd(0x0C)       #// Display On
    lcd_cmd(0x01)       #// Clear Display
    delay(2)
    
    Usb.println("end of lcd_begin");
end

#-------
# 液晶の文字全消去
#-------
def lcd_clear()
    lcd_cmd(0x01)   #//初期化コマンド出力
    delay(2)
end

#-------
# 液晶の文字列表示
#-------
def lcd_print(cs)
    cs.each_byte{|c| lcd_data(c)}
end

#-------
# プチ電車カーを止めます
#-------
def mStop()
    digitalWrite(Num[0],LOW)  #A1
    digitalWrite(Num[1],LOW)  #A2
    digitalWrite(Num[2],LOW)  #B1
    digitalWrite(Num[3],LOW)  #B2
end

#-------
# プチ電車カーを後退させます
#-------
def mUshiro()
  digitalWrite(Num[0],LOW)  #A1
  digitalWrite(Num[1],HIGH) #A2
  digitalWrite(Num[2],LOW)  #B1
  digitalWrite(Num[3],HIGH) #B2
  MaxVero.times do|i|
    delay 5
    pwm(Vero[0], i)  #モータの回転速度を設定する命令
    pwm(Vero[1], i)  #モータの回転速度を設定する命令
  end
end

#-------
# プチ電車カーを前進させます
#-------
def mMae()
  digitalWrite(Num[0],HIGH) #A1
  digitalWrite(Num[1],LOW)  #A2
  digitalWrite(Num[2],HIGH) #B1
  digitalWrite(Num[3],LOW)  #B2
  MaxVero.times do|i|
    delay 5
    pwm(Vero[0], i)  #モータの回転速度を設定する命令
    pwm(Vero[1], i)#モータの回転速度を設定する命令
  end
end

#-------
# プチ電車カーを左回転させます
#-------
def mLeft(t)
  digitalWrite(Num[0],HIGH) #A1
  digitalWrite(Num[1],LOW)  #A2
  digitalWrite(Num[2],LOW) #B1
  digitalWrite(Num[3],HIGH)  #B2
  MaxVero.times do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
  delay t
  MaxVero.step(0,-1) do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
end

#-------
# プチ電車カーを右回転させます
#-------
def mRight(t)
  digitalWrite(Num[0],LOW) #A1
  digitalWrite(Num[1],HIGH)  #A2
  digitalWrite(Num[2],HIGH) #B1
  digitalWrite(Num[3],LOW)  #B2
  MaxVero.times do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
  delay t
  MaxVero.step(0,-1) do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
end

#-----------------------------------------
Usb.println("System Start")
#lcd_begin()     #液晶の初期設定

#25.times do
#    len = analogRead(Sens)
#    lcd_setCursor(0,0)
#    lcd_print len.to_s
#    delay 500
#    len = analogRead(Sens)
#    lcd_setCursor(0,1)
#    lcd_print len.to_s
#    delay 500
#end
#System.exit

c = 0
randomSeed(micros())
mMae
while(true) do
    delay 50
    len = analogRead(Sens)
    #lcd_setCursor(0,0)
    #lcd_print len.to_s
    if(len<470)then
        led 1
        #mStop
        mUshiro
        delay 100
        #mStop
        if(random(2) == 0)then
            mRight MoveTime
            mLeft 5
        else
            mLeft MoveTime
            mRight 5
        end
        #mStop
        mMae
    end
    c = 1 - c
    led c
end

pwm(Vero[0], 0)
pwm(Vero[1], 0)
mStop
