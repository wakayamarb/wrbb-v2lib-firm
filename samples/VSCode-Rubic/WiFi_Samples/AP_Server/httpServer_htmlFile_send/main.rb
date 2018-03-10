#!mruby
#GR-CITRUS Version 2.42
WiFiEN = 5          #WiFiのEN:LOWでDisableです
HtmlFile = "WETHER1.HTM"
HeaderBL = ""
Header0 = "Content-Length: 0\r\n"
BtnFlg = false
Usb = Serial.new(0,115200)

pinMode(WiFiEN, OUTPUT) #WiFi_ENのピン設定

#ESP8266を一度停止させる(リセットと同じ)
pinMode(WiFiEN,OUTPUT)
digitalWrite(WiFiEN,LOW)   # LOW:Disable
delay 500
digitalWrite(WiFiEN,HIGH)   # HIGH:Enable
delay 500

if(!System.use?('SD'))then
  puts "SD Card can't use."
  System.exit
end

if(!System.use?('WiFi'))then
  puts "WiFi can't use."
  System.exit
end

tm1 = Rtc.getTime
delay 1100
tm2 = Rtc.getTime
if(tm1[5] == tm2[5] || tm1[0] < 2010)then
  puts 'RTC Initialized'
  Rtc.init
  Rtc.setTime([2018,3,3,2,30,00])
end

#****************************************
# WiFiをAPモードに設定します
#****************************************
def SetApMode()
  WiFi.disconnect
  WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
  WiFi.softAP "BigFileHTML 192.168.4.1","",2,0
  WiFi.dhcp 0,1
  WiFi.multiConnect 1
  WiFi.httpServer(-1)
end

#****************************************
# ヘッダを送信します
#****************************************
def headerSend(sesnum, bl)
  headly = "HTTP/1.1 200 OK\r\n"
  headly += "Server: GR-WA-MIKAN\r\n"
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
  blk = 384
  fn = SD.open(0, HtmlFile, 0)
  if(fn < 0)then
    puts "fn= " + fn.to_s
    return
  end
  size = SD.size(fn)
  cnt = (size / blk).floor
  puts cnt
  delay 0
  for j in 1..cnt
    body = ""
    delay 1
    for i in 1..blk
      body += SD.read(fn).chr
    end
    #puts body
    #puts j.to_s
    WiFi.send(sesnum, body)
    led
  end
  len = size - cnt * 256
  body = ""
  delay 0
  for i in 1..len
    body += SD.read(fn).chr
  end
  body += "\r\n"
  #Usb.print body
  WiFi.send(sesnum, body)
  SD.close(fn)
  puts HtmlFile + " send finish."
end


####### ここからプログラムスタート

SetApMode() # WiFiをAPモードに設定します

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
while(true)do
  res, sesnum = WiFi.httpServer
  #Usb.println res.to_s
  if(res == "/")then
    puts res + " " + sesnum.to_s + " /"
    headerSend(sesnum, HeaderBL)
    bodySend(sesnum)
  elsif(res == "0,CLOSED\r\n")
    puts res + " " + sesnum.to_s
  elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
    puts "Else(*,:" + res + " " + sesnum.to_s
  elsif(res != 0)
    puts "Else:" + res.to_s
    headerSend(sesnum, HeaderBL)
    bodySend(sesnum)
  end
  delay 5
end

WiFi.httpServer(-1)
WiFi.disconnect
