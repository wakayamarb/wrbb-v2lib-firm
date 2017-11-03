#!mruby
#Ver.2.33
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
#MaxVero = 120
MaxVero = 160
RotVero = 200
Rottime = 1000
RotPm = [RotVero, Rottime]
Vero = [4,10]
Num = [18,3,15,14]
Lev = [6,16]
Sens = 17            #アナログ距離センサ
WiFiEN = 5          #WiFiのEN:LOWでDisableです

Usb = Serial.new(0)
for i in Num do
    pinMode(i, OUTPUT)  #出力設定
end
pinMode(WiFiEN, OUTPUT) #WiFi_ENのピン設定
for i in Lev do
    pinMode(i, 2) #プルアップ  入力設定
end

#ESP8266を一度停止させる(リセットと同じ)
digitalWrite(WiFiEN, LOW)   #LOW:Disable
#delay 500
#digitalWrite(WiFiEN, HIGH)   #HIGH:Enable

#if(!System.use?('WiFi') || !System.use?('MP3', Lev))then
if(!System.use?('MP3', Lev))then
  Usb.println "SD Card or MP3 can't use."
 System.exit() 
end

#-------
# タンク動かします
# 0:停止, 1:前進, -1:後退
#-------
def movel(flgm)
  if(flgm == 1)then
    digitalWrite(Num[0],LOW)  #A1
    digitalWrite(Num[1],HIGH) #A2
    digitalWrite(Num[2],LOW)  #B1
    digitalWrite(Num[3],HIGH) #B2
  elsif(flgm == -1)
    digitalWrite(Num[0],HIGH)  #A1
    digitalWrite(Num[1],LOW)   #A2
    digitalWrite(Num[2],HIGH)  #B1
    digitalWrite(Num[3],LOW)   #B2
  else
    digitalWrite(Num[0],LOW)  #A1
    digitalWrite(Num[1],LOW)  #A2
    digitalWrite(Num[2],LOW)  #B1
    digitalWrite(Num[3],LOW)  #B2
    return
  end      
  p = 0

  while(p <= MaxVero)do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p += 1
    if(chkTurn)then
      return false
    end
  end
  return true
end
#-------
# タンクを、t ms回転させます
#-------
def rot(r0,r1,pm)
  led HIGH
  digitalWrite(Num[0],r1)  #A1
  digitalWrite(Num[1],r0)  #A2
  digitalWrite(Num[2],r0)  #B1
  digitalWrite(Num[3],r1)  #B2
  p = 0
  ps = 1
  (2 * pm[0]).times do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p += ps
    chkEnd()
    if(p == pm[0])then
      delay pm[1]
      ps = -1
    end
  end
end
#-------
# ボタンが押されたらプログラムモードに切り替わります
#-------
def chkEnd()
  if(digitalRead(Lev[0]) == 0)then
    pwm(Vero[0], 0)
    pwm(Vero[1], 0)
    digitalWrite(Num[0],HIGH)
    digitalWrite(Num[1],HIGH)
    digitalWrite(Num[2],HIGH)
    digitalWrite(Num[3],HIGH)
    System.exit
  end
end
#-------
# 回避するかどうか調べて、回避すべきであれば回避します
#-------
def chkTurn()
  chkEnd

  #if((analogRead(Sens) > 410)||(analogRead(Sens) < 200))then
  if((analogRead(Sens) > 380)||(analogRead(Sens) < 280))then
    #ランダムで右か左回転する
    ro = random(2)
    if ro == 0 then
      Usb.println "Right Rotation"
      robotalk 2
      rot(HIGH, LOW, RotPm)
    else
      Usb.println "Left Rotation"
      robotalk 3
      rot(LOW, HIGH, RotPm)
    end
    return true
  end
  return false
end

#-------
# しゃべります
#-------
def robotalk(flgt)
  pwm(Vero[0], 0)
  pwm(Vero[1], 0)
  if(flgt == 1)then
    MP3.play "mp3/mae.mp3"
  elsif(flgt == -1)
    MP3.play "mp3/ushiro.mp3"
  elsif(flgt == 2)
    MP3.play "mp3/right.mp3"
  elsif(flgt == 3)
    MP3.play "mp3/left.mp3"
  else
    MP3.play "mp3/stop.mp3"
  end      
end

#-----------------------------------------
Usb.println("RubiRobo System Start")

randomSeed(micros)
robotalk 1
while(movel(1) == false)do
  robotalk 1
end

while(true)do
  if(chkTurn)then
    robotalk 1
    while(movel(1) == false)do
      robotalk 1
    end
  end
  Usb.println analogRead(Sens).to_s
  delay 250
  led
end

robotalk 0
pwm(Vero[0], 0)
pwm(Vero[1], 0)
digitalWrite(Num[0],HIGH)
digitalWrite(Num[1],HIGH)
digitalWrite(Num[2],HIGH)
digitalWrite(Num[3],HIGH)

