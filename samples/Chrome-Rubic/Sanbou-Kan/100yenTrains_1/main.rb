#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 120
MoveTime = 1500
Vero = [4,10]
Num = [18,3,15,14]
Lev = [0,17]

Usb = Serial.new(0)
for i in Num do
    pinMode(i, OUTPUT)
end
for i in Lev do
    pinMode(i, 2) #プルアップ
end

#-------
# レバー状態状態を取得します
#-------
def lever()
  digitalRead(Lev[0]) + 2 * digitalRead(Lev[1])
end

#-------
# タンクを止めます
#-------
def mStop()
    digitalWrite(Num[0],LOW)  #A1
    digitalWrite(Num[1],LOW)  #A2
    digitalWrite(Num[2],LOW)  #B1
    digitalWrite(Num[3],LOW)  #B2
end
#-------
# タンク前進させます
#-------
def mUshiro()
  p = 0
  digitalWrite(Num[0],LOW)  #A1
  digitalWrite(Num[1],HIGH) #A2
  digitalWrite(Num[2],LOW)  #B1
  digitalWrite(Num[3],HIGH) #B2
  MaxVero.times do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p += 1
  end
end
#-------
# タンク後退させます
#-------
def mMae()
  p = 0
  digitalWrite(Num[0],HIGH) #A1
  digitalWrite(Num[1],LOW)  #A2
  digitalWrite(Num[2],HIGH) #B1
  digitalWrite(Num[3],LOW)  #B2
  MaxVero.times do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p += 1
  end
end
#-------
# タンクを、t ms回転させます
#-------
def mRot(r0,r1,t)
  led HIGH
  p = 0
  digitalWrite(Num[0],r0)  #A1
  digitalWrite(Num[1],r1)  #A2
  digitalWrite(Num[2],r1)  #B1
  digitalWrite(Num[3],r0)  #B2
  MaxVero.times do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p += 1
  end

  delay t
  
  MaxVero.times do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p -= 1
  end
end
#-----------------------------------------

if(lever == 0)then
    System.exit
end

Usb.println("System Start")

3.times do
    mMae
    delay MoveTime
    mUshiro
    delay MoveTime
    mRot(LOW, HIGH, MoveTime)
    mRot(HIGH, LOW, MoveTime)
end

pwm(Vero[0], 0)
pwm(Vero[1], 0)
digitalWrite(Num[0],HIGH)
digitalWrite(Num[1],HIGH)
digitalWrite(Num[2],HIGH)
digitalWrite(Num[3],HIGH)

