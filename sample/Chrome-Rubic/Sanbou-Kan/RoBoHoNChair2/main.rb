#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 80        #モータ速度速度の最大値を指定しています。0～255
MaxRotVero = 80     #モータ速度速度の最大値を指定しています。0～255
MoveTime = 450      #500msWaitさせたかったので用意している。
Vero = [4,10]       #モータの速度を決定するGR-CITRUSのピンが4番と10番です。     
Num = [18,3,15,14]  #モータの回転方向などを制御するビット、1モータ2ビットです。18,3番、15と14番がペアです
Sens = 17            #アナログ距離センサ
Lev = [0,16]        #ロボホンロボホンのレバー

Usb = Serial.new(0)
for i in Num do
 pinMode(i,OUTPUT)
end
for i in Lev do
    pinMode(i, 2) #プルアップ
end

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
  delay t
  MaxRotVero.step(0,-1) do |i|
    pwm(Vero[0], i)
    pwm(Vero[1], i)
  end
end

#-----------------------------------------
if(lever == 3)then
    System.exit
end

Usb.println("System Start")

lvr0 = 0
lvr1 = 0
cnt = 0
k = 0
while true do
  lvr0 = lever
  #Usb.println lvr0.to_s + "," + cnt.to_s

  if cnt == 4 then
    Usb.println lvr1.to_s
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
  led k
  k = 1 - k
end

mStop
System.exit

#mMae
#delay 500
#mStop
#mUshiro
#delay 500
#mStop
#System.exit

10.times do
    mRight 300
    delay 400
end

10.times do
    mLeft 300
    delay 400
end
mMae
delay 250
mStop
mUshiro
delay 250
mStop

#100.times do
#    len = analogRead(Sens)
#    Usb.println lever.to_s
#    Usb.println len.to_s
#    delay 600
#end
System.exit

maeFlg = true
cnt = 0
c = 0
randomSeed(micros())
while(true) do
    delay 50
    len = analogRead(Sens)  #410くらいをキープさせる
    while(len<390 && len>415)do
        maeFlg = false
        led 1
        if(random(2) == 0)then
            mRight MoveTime
            mLeft 5
        else
            mLeft MoveTime
            mRight 5
        end
        len = analogRead(Sens)  #410くらいをキープさせる
        cnt = 0
    end

    if(maeFlg == false)then
        mMae
        maeFlg = true
    end
    
#    if(len<400 || len>415)then
#        cnt += 1
#        if(cnt > 100)then
#            mUshiro
#            delay 500
#            cnt = 0
#        end
#    else 
#        cnt = 0
#    end
    c = 1 - c
    led c
end

pwm(Vero[0], 0)
pwm(Vero[1], 0)
mStop
