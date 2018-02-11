#!mruby
#GR-CITRUS Version 2.31

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
delay 500

Usb = Serial.new(0,115200)

if( !System.use?('SD') || !System.use?('WiFi'))then
    Usb.println "SD Card or WiFi can't use."
   System.exit() 
end
Usb.println "SD & WiFi Ready"

#-------
#urlのてデータを取得します
#-------
def getWiFi(tmpname, url )
  WiFi.disconnect
  WiFi.setMode 3  #Station-Mode & SoftAPI-Mode
  WiFi.connect("TAROSAY","37000")
  #WiFi.connect("000740DE0D79","")
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
    
    if(c == 0xd || c == 0xa)then
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
#Memにコピーします
#-------
def copyMem(src, dst)
  SD.open(0, src, 0)
  MemFile.open(0, dst, 2)
  c = SD.read(0)
  while(c >= 0)do
    MemFile.write(0, c.chr, 1 )
    c = SD.read(0)
  end
  MemFile.close(0)
  SD.close(0)
end
#-----------------------------------------
Usb.println("System Start")

#読み込むmrbファイルのあるURLを取得します
res = getWiFi("wifi.tmp", "tarosay.github.io/iotdl/data/geturl.txt")
if( !res )then
  Usb.println "Failed."
  System.reset    #自分をリセットします
end

body = getBody("geturl.txt", "wifi.tmp")

#mrbファイル名を取得します
mrbname = body[body.rindex("/") + 1..body.size]

#mrbファイルを取得します
res = getWiFi("wifi.tmp", body)
if( !res )then
  Usb.println "Failed."
  System.reset    #自分をリセットします
end

if(mrbname.size == 0)then
  Usb.println "Failed."
  System.reset    #自分をリセットします
end

Usb.println mrbname
getBody(mrbname, "wifi.tmp")  #mrbファイルを取り出します

copyMem(mrbname, mrbname) #Memにコピーします

#実行します
System.setrun mrbname




