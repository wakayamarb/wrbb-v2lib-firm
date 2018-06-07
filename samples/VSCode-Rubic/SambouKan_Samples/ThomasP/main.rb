#!mruby
#Ver2.48
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
ML = 0    #左のモーター
MR = 1    #右のモーター
Vero = [0,0]
Vpin = [4,10]
Cpin = [[18,3],[15,14]]
MaxErrCnt = 10
ErrCnt = 0
MaxDatInCnt = 1000
DatInCnt = 0
WiFiEN = 5          #WiFiのEN:LOWでDisableです
VL53Enable = false   #VL53L0Xを使うかどうか
IsConnectVL53 = false #VL53L0Xが接続されているかどうか
Distance = 80   #mm以下まで近づくと止まる
Usb = Serial.new(0,9600)
#Usb = Serial.new(0,115200)

pinMode(Cpin[ML][0], OUTPUT)  #出力設定
pinMode(Cpin[ML][1], OUTPUT)  #出力設定
pinMode(Cpin[MR][0], OUTPUT)  #出力設定
pinMode(Cpin[MR][1], OUTPUT)  #出力設定
pinMode(WiFiEN, OUTPUT) #WiFi_ENのピン設定

#ESP8266を一度停止させる(リセットと同じ)
digitalWrite(WiFiEN,LOW)   # LOW:Disable
#delay 500

if(System.use?("VL53L0X"))
  IsConnectVL53 = true
  VL53L0X.init(1) #I2Cの1番に接続します
  VL53L0X.startContinuous
end

#-------
# 前に障害物があるかどうか調べます
#-------
def sensorChk(p, tp)
  if(VL53Enable && IsConnectVL53 && VL53L0X.readContinuous < Distance)
    if(p > 0 && tp > 0)
      puts VL53L0X.readContinuous
      mMove(ML, 0)
      mMove(MR, 0)
      true
      return
    elsif(p == 0 && tp == 0)
      #puts VL53L0X.readContinuous
      if(digitalRead(Cpin[MR][0]) == LOW \
        && digitalRead(Cpin[MR][1]) == HIGH \
        && digitalRead(Cpin[ML][0]) == LOW \
        && digitalRead(Cpin[ML][1]) == HIGH)
        puts VL53L0X.readContinuous
        mMove(ML, 0)
        mMove(MR, 0)
        true
        return
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
  puts p.to_s + ", " + turn.to_s + ", " + tp.to_s
end

#-----------------------------------------
Usb.println("Thomas System Starting")

cmd = ""
while(true)do
  while(Usb.available > 0)do
    c = Usb.read
    for i in 0..(c.length - 1)
      cmd += c[i]
      Usb.print c[i]
      if(c[i] == "\r")

        if(cmd.length == 7)
          #コマンド処理します
          command(cmd)
          ErrCnt = 0
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