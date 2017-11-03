#!mruby
#Ver.2.33
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 150        #モータ速度速度の最大値を指定しています。0～255
MaxRotVero = 100     #モータ速度速度の最大値を指定しています。0～255
MoveTime = 450      #500msWaitさせたかったので用意している。
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
Usb.println("System Start")

mMae
led
delay 1500

mRight 1500
delay 500
mLeft 1500

mUshiro
led
delay 1500

mStop
