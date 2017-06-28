#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
#MaxVero = 120       #モータ速度速度の最大値を指定しています。0～255
MaxVero = 100       #モータ速度速度の最大値を指定しています。0～255
MoveTime = 450      #500msWaitさせたかったので用意している。
Vero = [4,10]       #モータの速度を決定するGR-CITRUSのピンが4番と10番です。     
Num = [18,3,15,14]  #モータの回転方向などを制御するビット、1モータ2ビットです。18,3番、15と14番がペアです
Sens = 17            #アナログ距離センサ

Usb = Serial.new(0)
for i in Num do
 pinMode(i,OUTPUT)
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

mMae
delay 500
mStop
mUshiro
delay 500
mStop
System.exit

mRight 5000
delay 1000
mLeft 5000
System.exit


#100.times do
#    len = analogRead(Sens)
#    Usb.println len.to_s
#    delay 600
#end
#System.exit

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
