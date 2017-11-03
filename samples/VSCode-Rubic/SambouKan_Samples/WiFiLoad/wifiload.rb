#!mruby
#GR-CITRUS Version 2.31
Lev = [1,16]        #ロボホンロボホンのレバー
WiFiEN = 5          #WiFiのEN:LOWでDisableです

for i in Lev do
    pinMode(i, 2) #INPUT & プルアップ
end
pinMode(WiFiEN, OUTPUT)

#ESP8266を一度停止させる(リセットと同じ)
digitalWrite(WiFiEN, LOW)   #LOW:Disable
delay 500
digitalWrite(WiFiEN, HIGH)   #HIGH:Enable

Usb = Serial.new(0,115200)

if(!System.use?('WiFi') || !System.use?('MP3', Lev))then
    Usb.println "SD Card or WiFi or MP3 can't use."
   System.exit() 
end
Usb.println "WiFi MP3 Ready"


#-------
#urlのデータを取得します
#-------
def getWiFi(tmpname, url )
  WiFi.disconnect
  WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
  WiFi.connect("TAROSAY","37000")
  WiFi.multiConnect 1
  if(WiFi.httpGetSD(tmpname, url) != 1)then
      WiFi.disconnect
      return false
  end
  WiFi.disconnect
  return true
end

#-------
#ボディを取り出します
#ボディの最初の1行を返します
#-------
def getBody(filename, tmpname)
  SD.open(0, tmpname, 0)
  c = SD.read(0)
  crlf = 0
  while(c >= 0)do
    if(c == 0xd || c == 0xa)then
      crlf += 1
      if(crlf == 4)then
          break
      end
    else
      crlf = 0
    end
    c = SD.read(0)
  end
  
  body = ""
  crlf = 0

  SD.open(1, filename, 2)
  c = SD.read(0)
  while(c >= 0)do
    #Usb.write(c.chr, 1)
    SD.write(1, c.chr, 1)
    
    if(c == 0xd || c == 0xa || body.size > 64)then
      crlf = 1
    end
    
    if(crlf == 0)then
      body += c.chr
    end
    
    c = SD.read(0)
  end
  SD.close(1)
  SD.close(0)
  return body
end

#-------
#ダウンロードに失敗したときの処理
#-------
def dlfalse()
  Usb.println "Failed."
  MP3.play "dlfalse.mp3"
  System.reset    #自分をリセットします
end
#-----------------------------------------
Usb.println("System Start")

MP3.led 1
MP3.play "wifiload.mp3"

#読み込むmrbファイルのあるURLを取得します
res = getWiFi("wifi.tmp", "tarosay.github.io/iotdl/data/geturl.txt")

if( !res )then
  dlfalse()
end

body = getBody("geturl.txt", "wifi.tmp")

#mrbファイル名を取得します
mrbname = body[body.rindex("/") + 1..body.size]

#mrbファイルを取得します
res = getWiFi("wifi.tmp", body)
if( !res )then
  dlfalse()
end

if(mrbname.size == 0)then
  dlfalse()
end

Usb.println mrbname
MP3.play "dlsccess.mp3"

#getBody(mrbname, "wifi.tmp")  #mrbファイルを取り出します
getBody('main.mrb', "wifi.tmp")  #mrbファイルを取り出します

MemFile.rm 'main.mrb'

#実行します
#System.setrun mrbname
System.reset
