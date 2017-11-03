#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 255       #モータ速度速度の最大値を指定しています。0～255
MoveTime = 1000      #500msWaitさせたかったので用意している。
Vero = [4,10]       #モータの速度を決定するGR-CITRUSのピンが4番と10番です。     
Num = [18,3,15,14]  #モータの回転方向などを制御するビット、1モータ2ビットです。18,3番、15と14番がペアです

Usb = Serial.new(0)
for i in Num do
 pinMode(i,OUTPUT)
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
  digitalWrite(Num[0],HIGH)  #A1
  digitalWrite(Num[1],LOW) #A2
  digitalWrite(Num[2],HIGH)  #B1
  digitalWrite(Num[3],LOW) #B2
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
  digitalWrite(Num[0],LOW) #A1
  digitalWrite(Num[1],HIGH)  #A2
  digitalWrite(Num[2],LOW) #B1
  digitalWrite(Num[3],HIGH)  #B2
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

1.times do
    mMae
    delay MoveTime
    mUshiro
    delay MoveTime
    mRight MoveTime
    mLeft MoveTime
end

pwm(Vero[0], 0)
pwm(Vero[1], 0)
mStop
