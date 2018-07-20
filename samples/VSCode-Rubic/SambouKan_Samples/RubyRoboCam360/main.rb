#!mruby
#Ver.2.51
#TB6612FNG L-L->STOP. L-H->CCW, H-L->CW, H-H->ShortBrake
#MaxVero = 120
MaxVero = 160
RotVero = 200
Rottime = 1000
RotPm = [RotVero, Rottime]
Vero = [4,10]
Num = [18,3,15,14]
Lev = [9,16,6]      #9,16はMP3用、6は切替えスイッチ用
Cam = 1             #カメラのシャッター
Sens = 17           #アナログ距離センサ
WiFiEN = 5          #WiFiのEN:LOWでDisableです
HtmlFile = "CONTROL.HTM"
HeaderBL = ""
Header0 = "Content-Length: 0\r\n"
BtnFlg = false
VideoFlg = false
PicTime = 0

for i in Num do
  pinMode(i, OUTPUT)  #出力設定
end
pinMode(WiFiEN, OUTPUT) #WiFi_ENのピン設定
for i in Lev do
  pinMode(i, INPUT_PULLUP) #プルアップ  入力設定
end
pinMode(Cam, OUTPUT) #カメラのシャッターのサーボセット

#ESP8266を一度停止させる(リセットと同じ)
pinMode(WiFiEN, OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
#digitalWrite(5,HIGH)   # HIGH:Enable
#delay 500

System.exit "SD Card can't use." if(!System.use?('SD'))
System.exit "MP3 can't use." if(!System.use?('MP3', Lev))

#****************************************
# シャッターチャンスを選びます
#****************************************
class Shutter
  #attr_accessor :ntim # インスタンス変数に対応するゲッタとセッタを定義
  def initialize(span)   # コンストラクタ
    @Span = span
    @ntim = millis + @Span * 1000
  end

  def chance?()
    if(@ntim < millis)then
      @ntim = millis + @Span * 1000
      return true
    end
    return false
  end
end

#****************************************
# 360度 写真を撮ります 0:写真 1:ビデオ
#****************************************
def camera(m)
  MP3.play "mp3/camera.mp3" if(m == 0)
  MP3.play "mp3/video.mp3" if(m == 1)
  delay 0
  Servo.attach(0, Cam)
  Servo.write(0,10)
  if(m == 1)
    delay 600
  else
    delay 200
  end
  Servo.write(0, 20)
  Servo.detach(0)
  delay 500
end

#****************************************
# WiFiをAPモードに設定します
#****************************************
def setApMode()
  WiFi.disconnect
  WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
  WiFi.softAP "RubyRoboCam360 192.168.4.1","",2,0
  WiFi.dhcp 0,1
  WiFi.multiConnect 1
  WiFi.httpServer(-1)
end

#****************************************
# ヘッダを送信します
#****************************************
def headerSend(sesnum, bl)
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
    puts("fn= " + fn.to_s)
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
    #putsbody
    #puts j
    WiFi.send(sesnum, body)
  end
  len = size - cnt * 256
  body = ""
  delay 0
  for i in 1..len
    body += SD.read(fn).chr
  end
  #body += "\r\n"
  #puts body
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
  robotalka flgm
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
  #if((analogRead(Sens) > 410)||(analogRead(Sens) < 200))then
  if((analogRead(Sens) > 380)||(analogRead(Sens) < 280))then
    #ランダムで右か左回転する
    ro = random(2)
    if ro == 0 then
      puts "Right Rotation"
      robotalk 2
      rot(HIGH, LOW, RotPm)
    else
      puts "Left Rotation"
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
  if(BtnFlg == true)then
    return
  end
  pwm(Vero[0], 0)
  pwm(Vero[1], 0)
  caflg = false

  #470を越えているときには写真を撮る
  if(analogRead(Sens) > 470)then
    camera 0
    caflg = true
  end

  if(flgt == 1 && caflg == false)then
    MP3.play "mp3/mae.mp3"
  elsif(flgt == -1)
    MP3.play "mp3/ushiro.mp3"
  elsif(flgt == 2)
    MP3.play "mp3/right.mp3"
  elsif(flgt == 3)
    MP3.play "mp3/left.mp3"
  elsif(caflg == false)
    MP3.play "mp3/stop.mp3"
  end
end

#-------
# しゃべります
#-------
def robotalka(flgt)
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
puts("Ruby Cam-Robo360 Starting")

TkPic = Shutter.new(120)

BtnFlg = false
randomSeed(micros)
robotalk 1
movel 1

while(true)do
  if(chkTurn)then
    robotalk 1
    while(movel(1) == false)do
      robotalk 1
      if(BtnFlg == true)then
        break
      end
      #写真を撮るタイミングになったら写真を撮ります
      if(TkPic.chance?)then
        movel 0
        camera 0
      end        
    end
  end

  #写真を撮るタイミングになったら写真を撮ります
  if(TkPic.chance?)then
    movel 0
    camera 0
    movel 1
  end        

  if(BtnFlg == true)then
    break
  end

  puts analogRead(Sens)
  delay 250
  led
end

#止まります
movela(0)

MaxVero = 100
RotVero = 100
Rottime = 2000
RotPm = [RotVero, Rottime]

#ESP8266をEnableにする
digitalWrite(5, HIGH)   # HIGH:Enable
delay 500

#WiFiをクラスを使います
System.exit "WiFi can't use." if(!System.use?('WiFi'))

setApMode() # WiFiをAPモードに設定します

#HeaderBLを生成します
fn = SD.open(0,HtmlFile,0)
if(fn < 0)then
  puts "fn= " + fn.to_s
  System.exit
end
HeaderBL = "Content-Length: " + SD.size(fn).to_s + "\r\n"
SD.close(fn)

#httpサーバを開始します
WiFi.httpServer(80)

BtnFlg = false

runmode = 0
puts "Changed Manual Control"

while(true)do
  res, sesnum = WiFi.httpServer
  #puts res
  if(res == "/")then
    puts res + " " + sesnum.to_s + " /"
    headerSend(sesnum, HeaderBL)
    bodySend(sesnum)
  elsif(res == "/?front=1")
    puts res + " " + sesnum.to_s + " f"
    headerSend(sesnum, Header0)
    runmode = 1
    movela(1)
  elsif(res == "/?stop=1")
    puts res + " " + sesnum.to_s + " s"
    headerSend(sesnum, Header0)
    runmode = 0
    movela(0)
  elsif(res == "/?back=1")
    puts res + " " + sesnum.to_s + " b"
    headerSend(sesnum, Header0)
    runmode = -1
    movela(-1)
  elsif(res == "/?left=1")
    puts res + " " + sesnum.to_s + " l"
    headerSend(sesnum, Header0)
    robotalka 3
    if(runmode == 0)then
      rota(LOW, HIGH, RotPm)
    else
      rot(LOW, HIGH, RotPm)
      movela runmode
    end
  elsif(res == "/?right=1")
    puts res + " " + sesnum.to_s + " r"
    headerSend(sesnum, Header0)
    robotalka 2
    if(runmode == 0)then
      rota(HIGH, LOW, RotPm)
    else
      rot(HIGH, LOW, RotPm)
      movela runmode
    end
  elsif(res == "/?camera=1")
    puts res + " " + sesnum.to_s + " c"
    headerSend(sesnum, Header0)
    if(runmode != 0)then
      runmode = 0
      movela(0)
    end
    camera 0
    VideoFlg = false
  elsif(res == "/?video=1")
    puts res + " " + sesnum.to_s + " v"
    headerSend(sesnum, Header0)
    if(runmode != 0)then
      runmode = 0
      movela(0)
    end
    if(VideoFlg)then
      camera 2  #ビデオ撮影を止めます
      VideoFlg = false
    else
      camera 1
      VideoFlg = true
    end
  elsif(res == "0,CLOSED\r\n")
    puts res + " " + sesnum.to_s

  elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
    puts "Else(*,:" + res + " " + sesnum.to_s
  
  elsif(res != 0)
    puts "Else:" + res.to_s
    headerSend(sesnum, HeaderBL)
    bodySend(sesnum)
  end
  
  chkEnd()
  if(BtnFlg == true)then
    break
  end

  if(VideoFlg == false)then
    #写真を撮るタイミングになったら写真を撮ります
    camera 0 if(TkPic.chance?)
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
