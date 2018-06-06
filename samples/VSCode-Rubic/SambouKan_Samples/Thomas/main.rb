#!mruby
#Ver2.48
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
ML = 0    #左のモーター
MR = 1    #右のモーター
Vero = [0,0]
Vpin = [4,10]
Cpin = [[18,3],[15,14]]
WiFiEN = 5          #WiFiのEN:LOWでDisableです
#Pvero = [0,0]   #PWMの値

Usb = Serial.new(0,115200)
Rpi = Serial.new(3,115200)

pinMode(Cpin[ML][0], OUTPUT)  #出力設定
pinMode(Cpin[ML][1], OUTPUT)  #出力設定
pinMode(Cpin[MR][0], OUTPUT)  #出力設定
pinMode(Cpin[MR][1], OUTPUT)  #出力設定
pinMode(WiFiEN, OUTPUT) #WiFi_ENのピン設定
#for i in Lev do
#    pinMode(i, 2) #プルアップ  入力設定
#end

#ESP8266を一度停止させる(リセットと同じ)
digitalWrite(WiFiEN,LOW)   # LOW:Disable
#delay 500

#System.exit("WA-MIKAN can't use.") if(!System.use?("WiFi"))
#System.exit("SD can't use.") if(!System.use?("SD"))
#System.exit("MP3 can't use.") if(!System.use?("MP3"))


#-------
# 止めます
# 停止 st= 0 | 1
# 0:ストップ(出力ハイインピー) IN1,IN2,PWM = L,L,L)
# 1:ショートブレーキ(出力LOW,LOW) IN1,IN2,PWM = H,H,L
#-------
def mStop(flgm)
  if(flgm == 0)then
    digitalWrite(Cpin[ML][0],LOW) #A1
    digitalWrite(Cpin[ML][1],LOW) #A2
    digitalWrite(Cpin[MR][0],LOW) #B1
    digitalWrite(Cpin[MR][1],LOW) #B2
    Vero[ML] = 0
    Vero[MR] = 0
    pwm(Vpin[ML], Vero[ML])
    pwm(Vpin[MR], Vero[MR])
  else
    digitalWrite(Cpin[ML][0],HIGH) #A1
    digitalWrite(Cpin[ML][1],HIGH) #A2
    digitalWrite(Cpin[MR][0],HIGH) #B1
    digitalWrite(Cpin[MR][1],HIGH) #B2
    Vero[ML] = 0
    Vero[MR] = 0
    pwm(Vpin[ML], Vero[ML])
    pwm(Vpin[MR], Vero[MR])
  end      
end
#-------
# 前進後退させます
# 0:停止, 1:前進, -1:後退
#-------
def mMove(flgm,p)
  if(flgm == 1)then
    digitalWrite(Cpin[ML][0],LOW) #A1
    digitalWrite(Cpin[ML][1],HIGH) #A2
    digitalWrite(Cpin[MR][0],LOW) #B1
    digitalWrite(Cpin[MR][1],HIGH) #B2
    Vero[ML] = p
    Vero[MR] = p
    pwm(Vpin[ML], Vero[ML])
    pwm(Vpin[MR], Vero[MR])
  elsif(flgm == -1)
    digitalWrite(Cpin[ML][0],HIGH) #A1
    digitalWrite(Cpin[ML][1],LOW) #A2
    digitalWrite(Cpin[MR][0],HIGH) #B1
    digitalWrite(Cpin[MR][1],LOW) #B2
    Vero[ML] = p
    Vero[MR] = p
    pwm(Vpin[ML], Vero[ML])
    pwm(Vpin[MR], Vero[MR])
  else
    mStop 1
  end      
end
#-------
# カーブさせます
# 0:右カーブ
# 1:左カーブ
#-------
def mTern(flgm,p)
  if(flgm == 0)then
    Vero[MR] = Vero[MR] * p / 100
    pwm(Vpin[MR], Vero[MR])
  else
    Vero[ML] = Vero[ML] * p / 100
    pwm(Vpin[ML], Vero[ML])
  end      
end
#-------
# 超信地旋回します
# 0:右旋回
# 1:左旋回
#-------
def mSwing(flgm,p)
  mStop 0
  if(flgm == 0)then
    digitalWrite(Cpin[ML][0],LOW) #A1
    digitalWrite(Cpin[ML][1],HIGH) #A2
    digitalWrite(Cpin[MR][0],HIGH) #B1
    digitalWrite(Cpin[MR][1],LOW) #B2
  else
    digitalWrite(Cpin[ML][0],HIGH) #A1
    digitalWrite(Cpin[ML][1],LOW) #A2
    digitalWrite(Cpin[MR][0],LOW) #B1
    digitalWrite(Cpin[MR][1],HIGH) #B2
  end
  Vero[ML] = p
  Vero[MR] = p
  pwm(Vpin[ML], Vero[ML])
  pwm(Vpin[MR], Vero[MR])
end

#-------
# 前進 af=0  (値は速度で 0～255)
# 後退 bb=0 (値は速度で 0～255)
# 停止 st=0 ( 0:ストップ(出力ハイインピー)、1:ショートブレーキ(出力LOW,LOW))
# 右曲がり tr=0 (値は右タイヤの速度比率、0～100、現在の速度から比率分速度を下げる)
# 左曲がり tl=0 (左タイヤで、同上)
# 超信地旋回　右 sr=0 (値は速度で 1～255)
# 超信地旋回　左 sl=0 (値は速度で 1～255)
# 
# センサ感度 dt=10 (値は感度範囲距離)
# センサ許可 se=0 (0:無効, 1:有効)
#-------
def command(cmd)
  puts cmd
  if(cmd[0..1] == "af")
    p = cmd[3..cmd.size-2].to_i
    puts "前進:" + p.to_s
    mMove(1,p)
  elsif(cmd[0..1] == "bb")
    p = cmd[3..cmd.size-2].to_i
    puts "後退:" + p.to_s
    mMove(-1,p)
  elsif(cmd[0..1] == "tr")
    p = cmd[3..cmd.size-2].to_i
    puts "右回り:" + p.to_s
    mTern(0,p)
  elsif(cmd[0..1] == "tl")
    p = cmd[3..cmd.size-2].to_i
    puts "左回り:" + p.to_s
    mTern(1,p)
  elsif(cmd[0..1] == "sr")
    p = cmd[3..cmd.size-2].to_i
    puts "右超信地旋回:" + p.to_s
    mSwing(0,p)
  elsif(cmd[0..1] == "sl")
    p = cmd[3..cmd.size-2].to_i
    puts "左超信地旋回:" + p.to_s
    mSwing(1,p)
  elsif(cmd[0..1] == "st")
    p = cmd[3..cmd.size-2].to_i
    puts "停止:" + p.to_s
    mStop(p)
  else
    puts "エラー"
    mStop(1)
  end
end

#-----------------------------------------
Usb.println("Thomas System Starting")

cmd = ""
while(true)do
  while(Usb.available > 0)do
    c = Usb.read
    for i in 0..(c.length - 1)
      cmd += c[i]
      if(c[i] == "\r")
        #コマンド処理します
        command(cmd)
        cmd = ""  
      end
    end
  end
  delay 0
end