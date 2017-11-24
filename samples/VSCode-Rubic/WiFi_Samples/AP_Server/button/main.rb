#!mruby
#GR-CITRUS Version 2.35
HtmlFile = "CONTROL.HTM"
HeaderBL = ""
Header0 = "Content-Length: 0\r\n"

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # HIGH:Enable
delay 500

Usb = Serial.new(0,115200)

if(!System.use?('SD'))then
  Usb.println "SD Card can't use."
  System.exit() 
end
if(!System.use?('WiFi'))then
  Usb.println "WiFi can't use."
  System.exit() 
end
#Usb.println "WiFi Ready"

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
  WiFi.softAP "RubyRobo 192.168.4.1","",2,3
  
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
  headly = "HTTP/1.1 200 OK\r\n"
  headly += "Server: GR-CITRUS\r\n"
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

#----------------------------------------------------
Usb.println "Ruby Robo System Starting."

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
#Usb.println WiFi.httpServer(80).to_s
WiFi.httpServer(80)

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
  elsif(res == "/?stop=1")
    Usb.println res + " " + sesnum.to_s + " s"
    headerSend(sesnum, Header0)
  elsif(res == "/?back=1")
    Usb.println res + " " + sesnum.to_s + " b"
    headerSend(sesnum, Header0)
  elsif(res == "/?left=1")
    Usb.println res + " " + sesnum.to_s + " l"
    headerSend(sesnum, Header0)
  elsif(res == "/?right=1")
    Usb.println res + " " + sesnum.to_s + " r"
    headerSend(sesnum, Header0)
    break
  elsif(res == "0,CLOSED\r\n")
    Usb.println res + " " + sesnum.to_s

  elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
    Usb.println "Else(*,:" + res + " " + sesnum.to_s
  
  elsif(res != 0)
    Usb.println "Else:" + res.to_s
    headerSend(sesnum, HeaderBL)
    bodySend(sesnum)
  end
  delay 0
end

WiFi.httpServer(-1)
WiFi.disconnect