#!mruby
#Ver.2.27
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 120
Rottime = 1500
Vero = [4,10]
Num = [18,3,15,14]
Lev = [0,16]
Sens = 17            #アナログ距離センサ

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
def mstop()
    digitalWrite(Num[0],LOW)  #A1
    digitalWrite(Num[1],LOW)  #A2
    digitalWrite(Num[2],LOW)  #B1
    digitalWrite(Num[3],LOW)  #B2
end
#-------
# タンク前進させます
#-------
def mstart()
  p = 0
  digitalWrite(Num[0],HIGH)  #A1
  digitalWrite(Num[1],LOW)   #A2
  digitalWrite(Num[2],HIGH)  #B1
  digitalWrite(Num[3],LOW)   #B2
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
def rot(r0,r1,t)
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

cons = [0,0,0,0] #front,left,right,break
moveFlg = 0
cnt = 0
k = 1
while true do
  lvr = lever
  sc = cons[lvr] + 1
  for i in 0..3 do
    if cons[i] == 4 then
      if(i == 0)then
        if(moveFlg != 1)then
          moveFlg = 1
          Usb.println "Start"
          mstart
        elsif moveFlg != 0 then
          moveFlg = 0
          Usb.println "STOP"
          mstop
        end
      elsif i == 1 then
        Usb.println "Left Rotation"
        rot(HIGH, LOW, Rottime)
        if(moveFlg == 1)then
          mstart
        else
          mstop
        end
      elsif i == 2 then
        Usb.println "Right Rotation"
        rot(LOW, HIGH, Rottime)
        if(moveFlg == 1)then
          mstart
        else
          mstop
        end
      end
    end
    cons[i] = 0
  end
  cons[lvr] = sc
  #Usb.println cnt.to_s
  5.times do
    delay 50
    if(analogRead(Sens) > 420)then
      moveFlg = 0
      Usb.println "STOP"
      mstop
      break
    end
  end

  if lvr != 3 then
    if cnt > 6 then break end
    cnt += 1
  else
    cnt = 0
  end
  led k
  k = 1 - k
end

pwm(Vero[0], 0)
pwm(Vero[1], 0)
digitalWrite(Num[0],HIGH)
digitalWrite(Num[1],HIGH)
digitalWrite(Num[2],HIGH)
digitalWrite(Num[3],HIGH)

