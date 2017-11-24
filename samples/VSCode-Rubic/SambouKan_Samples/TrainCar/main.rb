#!mruby
#Ver.2.35
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
MaxVero = 130
RotVero = 110
Rottime = 100
RotPm = [RotVero, Rottime]
Vero = [4,10]
Num = [18,3,15,14]
Lev = [9,16,0]
Sens = 17            #アナログ距離センサ
WiFiEN = 5          #WiFiのEN:LOWでDisableです
HtmlFile = "CONTROL.HTM"
HeaderBL = ""
Header0 = "Content-Length: 0\r\n"
BtnFlg = false
Usb = Serial.new(0,115200)

for i in Num do
    pinMode(i, OUTPUT)  #出力設定
end
pinMode(WiFiEN, OUTPUT) #WiFi_ENのピン設定
for i in Lev do
    pinMode(i, 2) #プルアップ  入力設定
end

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
#digitalWrite(5,HIGH)   # HIGH:Enable
#delay 500

if(!System.use?('SD'))then
  Usb.println "SD Card can't use."
  System.exit 
end

#****************************************
# WiFiをAPモードに設定します
#****************************************
def SetApMode()
  #Usb.println "WiFi disconnect"
  #Usb.println WiFi.disconnect
  WiFi.disconnect

  #Usb.println "WiFi Mode Setting"
  #Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
  WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
  
  #Usb.println "WiFi access point"
  #Usb.println WiFi.softAP "RubyRobo 192.168.4.1","37003700",2,3
  WiFi.softAP "TrainCar 192.168.4.1","",3,0
  
  #Usb.println "WiFi dhcp enable"
  #Usb.println WiFi.dhcp 0,1
  WiFi.dhcp 0,1
  
  #Usb.println "WiFi ipconfig"
  #Usb.println WiFi.ipconfig

  #Usb.println "WiFi multiConnect Set"
  #Usb.println WiFi.multiConnect 1
  WiFi.multiConnect 1

  #httpサーバーモードを止めます
  #Usb.println WiFi.httpServer(-1).to_s
  WiFi.httpServer(-1)
end

#****************************************
# ヘッダを送信します
#****************************************
def headerSend(sesnum, bl)
  led 0
  headly = "HTTP/1.1 200 OK\r\n"
  headly += "Server: RubyRobo\r\n"
  headly += "Content-Type: text/html\r\n"
  #headly += "Date: Sun, 22 Oct 2017 12:00:00 GMT\r\n"
  headly += "Connection: close\r\n"
  headly += bl
  headly += "\r\n"
  WiFi.send(sesnum, headly)
end

#****************************************
# ボディを送信します
#****************************************
def bodySend(sesnum)
  fn = SD.open(0, HtmlFile, 0)
  if(fn < 0)then
    Usb.println "fn= " + fn.to_s
    return
  end
  size = SD.size(fn)
  cnt = (size / 256).floor
  for j in 1..cnt
    body = ""
    #delay 0
    for i in 1..256
      body += SD.read(fn).chr
    end
    #Usb.print body
    #Usb.println j.to_s
    WiFi.send(sesnum, body)
  end
  len = size - cnt * 256
  body = ""
  delay 0
  for i in 1..len
    body += SD.read(fn).chr
  end
  #body += "\r\n"
  #Usb.print body
  WiFi.send(sesnum, body)
  SD.close(fn)
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
    if(flgm == 1)then
      if(chkTurn)then
        return false
      end
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
    chkEnd
    if(BtnFlg == true)then
      return
    end
  
    if(p == pm[0])then
      delay pm[1]
      ps = -1
    end
  end
end
#-------
# タンク動かします
# 0:停止, 1:前進, -1:後退
#-------
def movela(flgm)
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
  end      
  p = 0
  while(p <= MaxVero)do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p += 1
  end
end
#-------
# タンクを回転させます
#-------
def rota(r0,r1,pm)
  led HIGH
  digitalWrite(Num[0],r1)  #A1
  digitalWrite(Num[1],r0)  #A2
  digitalWrite(Num[2],r0)  #B1
  digitalWrite(Num[3],r1)  #B2
  p = 0
  pm[0].times do
    delay 5
    pwm(Vero[0], p)
    pwm(Vero[1], p)
    p += 1
    chkEnd
    if(BtnFlg == true)then
      return
    end

  end
end
#-------
# ボタンが押されたらプログラムモードに切り替わります
#-------
def chkEnd()
  while(digitalRead(Lev[2]) == 0)do
    pwm(Vero[0], 0)
    pwm(Vero[1], 0)
    digitalWrite(Num[0],HIGH)
    digitalWrite(Num[1],HIGH)
    digitalWrite(Num[2],HIGH)
    digitalWrite(Num[3],HIGH)
    BtnFlg = true
  end
end
#-------
# 回避するかどうか調べて、回避すべきであれば回避します
#-------
def chkTurn()
  chkEnd
  if(BtnFlg == true)then
    return true
  end
  if((analogRead(Sens) > 450)||(analogRead(Sens) < 300))then
  #if((analogRead(Sens) > 380)||(analogRead(Sens) < 280))then
    #ランダムで右か左回転する
    ro = random(2)
    if ro == 0 then
      Usb.println "Right Rotation"
      rot(HIGH, LOW, RotPm)
    else
      Usb.println "Left Rotation"
      rot(LOW, HIGH, RotPm)
    end
    return true
  end
  return false
end
#-----------------------------------------
Usb.println("TrainCar System Starting")

BtnFlg = false
randomSeed(micros)
while(movel(1) == false)do
  if(BtnFlg == true)then
    break
  end
end

while(true)do
  if(chkTurn)then
    while(movel(1) == false)do
      if(BtnFlg == true)then
        break
      end
    end
  end

  if(BtnFlg == true)then
    break
  end

  Usb.println analogRead(Sens).to_s
  delay 250
  led
end

#止まります
movela(0)

MaxVero = 70
RotVero = 80
Rottime = 100
RotPm = [RotVero, Rottime]

#ESP8266をEnableにする
digitalWrite(5, HIGH)   # HIGH:Enable
delay 500

#WiFiをクラスを使います
if(!System.use?('WiFi'))then
  Usb.println "WiFi can't use."
  System.exit 
end

SetApMode() # WiFiをAPモードに設定します

#HeaderBLを生成します
fn = SD.open(0,HtmlFile,0)
if(fn < 0)then
  Usb.println "fn= " + fn.to_s
  System.exit
end
HeaderBL = "Content-Length: " + SD.size(fn).to_s + "\r\n"
SD.close(fn)

#httpサーバを開始します
WiFi.httpServer(80)

BtnFlg = false
ledcnt = 0
runmode = 0
#自動操縦モード
Usb.println "Changed Manual Control"
while(true)do
  res, sesnum = WiFi.httpServer
  #Usb.println res.to_s
  if(res == "/")then
    Usb.println res + " " + sesnum.to_s + " /"
    headerSend(sesnum, HeaderBL)
    bodySend(sesnum)
  elsif(res == "/?front=1")
    Usb.println res + " " + sesnum.to_s + " f"
    headerSend(sesnum, Header0)
    runmode = 1
    movela(1)
  elsif(res == "/?stop=1")
    Usb.println res + " " + sesnum.to_s + " s"
    headerSend(sesnum, Header0)
    runmode = 0
    movela(0)
  elsif(res == "/?back=1")
    Usb.println res + " " + sesnum.to_s + " b"
    headerSend(sesnum, Header0)
    runmode = -1
    movela(-1)
  elsif(res == "/?right=1")
    Usb.println res + " " + sesnum.to_s + " l"
    headerSend(sesnum, Header0)
    if(runmode == 0)then
      rota(LOW, HIGH, RotPm)
    else
      rot(LOW, HIGH, RotPm)
      movela runmode
    end
  elsif(res == "/?left=1")
    Usb.println res + " " + sesnum.to_s + " r"
    headerSend(sesnum, Header0)
    if(runmode == 0)then
      rota(HIGH, LOW, RotPm)
    else
      rot(HIGH, LOW, RotPm)
      movela runmode
    end
  elsif(res == "0,CLOSED\r\n")
    Usb.println res + " " + sesnum.to_s

  elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
    Usb.println "Else(*,:" + res + " " + sesnum.to_s
  
  elsif(res != 0)
    Usb.println "Else:" + res.to_s
    headerSend(sesnum, HeaderBL)
    bodySend(sesnum)
  end
  ledcnt += 1
  if(ledcnt > 50)then
    led
    ledcnt = 0
  end
  chkEnd()
  if(BtnFlg == true)then
    break
  end
  delay 0
end

movela(0)
digitalWrite(Num[0],HIGH)
digitalWrite(Num[1],HIGH)
digitalWrite(Num[2],HIGH)
digitalWrite(Num[3],HIGH)
WiFi.httpServer(-1)
WiFi.disconnect
