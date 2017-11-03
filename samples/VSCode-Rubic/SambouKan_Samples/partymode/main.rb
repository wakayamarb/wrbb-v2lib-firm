#!mruby
#Ver.2.31
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 150        #モータ速度速度の最大値を指定しています。0～255
MaxRotVero = 100     #モータ速度速度の最大値を指定しています。0～255
MoveTime = 450      #500msWaitさせたかったので用意している。
Vero = [4,10]       #モータの速度を決定するGR-CITRUSのピンが4番と10番です。
Num = [18,3,15,14]  #モータの回転方向などを制御するビット、1モータ2ビットです。18,3番、15と14番がペアです
Sens = 17           #アナログ距離センサ
Lev = [1,16]        #ロボホンロボホンのレバー
WiFiEN = 5          #WiFiのEN:LOWでDisableです

Usb = Serial.new(0)
for i in Num do
  pinMode(i,OUTPUT)
end
for i in Lev do
  pinMode(i, 2) #プルアップ
end
pinMode(WiFiEN, OUTPUT)

#WiFiをDisableにします
digitalWrite(WiFiEN, LOW)
    
#-------
# レバー状態状態を取得します
#-------
def lever()
  3 - digitalRead(Lev[0]) - digitalRead(Lev[1]) - digitalRead(Lev[1])
end

#-------
# プチ電車カーを止めます
#-------
def mStop()
    pwm(Vero[0], 0)
    pwm(Vero[1], 0)
    digitalWrite(Num[0],LOW)  #A1
    digitalWrite(Num[1],LOW)  #A2
    digitalWrite(Num[2],LOW)  #B1
    digitalWrite(Num[3],LOW)  #B2
end

#-------
# プチ電車カーを前進させます
#-------
def mMae()
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
# プチ電車カーを後退させます
#-------
def mUshiro()
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
  MaxRotVero.times do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
  
  if(t <= 0)then
      return
  end
  delay t
  MaxRotVero.step(0,-1) do |i|
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

  MaxRotVero.times do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
  if(t <= 0)then
      return
  end
  delay t
  MaxRotVero.step(0,-1) do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
end

#-----------------------------------------
if(lever == 3)then System.exit end

Usb.println("System Start")

mStop
randomSeed(micros())
lvr0 = 0
lvr1 = 0
cnt = 0
k = 241
while true do
  lvr0 = lever
  #Usb.println lvr0.to_s + "," + cnt.to_s + "," + k.to_s
  
  if(k > 180)then
    tm = random(1,5)
    rot = random(2)
    k = 0
    mStop
    if(rot == 0)then
        mRight tm * 800
    else
        mLeft tm * 800
    end
  end
  
  if cnt == 3 then
    if(lvr1 == 1)then
      mStop
      #mLeft 500
    elsif(lvr1 == 2)then
      mStop
      #mRight 500
    elsif(lvr1 == 3)then
      mStop
      #System.setrun "wifiload.mrb"
      #System.exit    
    end
  end
  lvr1 = lvr0
  
  5.times do
    delay 50
  end

  if lvr0 > 0 then
    if cnt > 6 then break end
    cnt += 1
  else
    cnt = 0
  end
  led k % 2
  k += 1
end

mStop
System.exit
