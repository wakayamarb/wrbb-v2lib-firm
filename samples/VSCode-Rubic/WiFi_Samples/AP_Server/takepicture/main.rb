#!mruby
#GR-CITRUS Version 2.35
#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # LOW:Disable
delay 500

Usb = Serial.new(0,115200)
Cam = Serial.new(1,115200)

if(!System.use?('SD'))then
  Usb.println "SD Card can't use."
  System.exit() 
end

if(!System.use?('WiFi'))then
  Usb.println "WiFi can't use."
  System.exit() 
end

tm = Rtc.getTime
if(tm[0] < 2010)then
  Usb.println 'RTC Initialized'
  Rtc.init
end

#/// コマンド送信
def txcmd(cmd)
  for i in 0..(cmd.length - 1) do
    Usb.print cmd.bytes[i].to_s(16) + " "
  end
  Usb.println ""
  Cam.write(cmd,cmd.length)
end
#/// バッファクリア
def clrbuf()
  while(Cam.available()>0) do
    Cam.read()
  end
end

#// カメラとの通信を確認します
#//  1.カメラに initdata送信 → ACK受信
#//  2.カメラから、initdata受信 → ACK送信
#// これが出来れば、確認完了です
def init()
  initdata = "\xAA\x0D\x00\x00\x00\x00"
  ackdata = "\xAA\x0E\x0D\x00\x00\x00"
  initArry = [ackdata, initdata]
  iniCnt = 0
  clrbuf()
  txcmd(initdata)
  txcmd(initdata)

  rxchr = ""
  atm = millis + 1000
  syncStop = millis + 10000
  res = false
  while(syncStop > millis) do
    if(Cam.available() > 0) then
      rxchr += Cam.read()
      if(rxchr.length >= 6)then
        if(rxchr[0,6] == initArry[iniCnt])then
        #if(rxchr.bytes[0] == 0xAA \
        #  and rxchr.bytes[1] == 0x0E \
        #  and rxchr.bytes[2] == 0x0D \
        #  and rxchr.bytes[4] == 0x00 \
        #  and rxchr.bytes[5] == 0x00 \
        #  )then
          if(iniCnt == 1)then
            txcmd(ackdata)
            res = true
            break
          end

          Usb.println "ACK OK. Wait initdata."
          iniCnt = 1
          rxchr = ""
          atm = millis + 2000
        else
          clrbuf()
          iniCnt = 0
          rxchr = ""
          Usb.println "Error Sync ReTry"
          txcmd(initdata)
          atm = millis + 2000
        end
      end
    end

    if(atm < millis)then
      clrbuf()
      iniCnt = 0
      rxchr = ""
      txcmd(initdata)
      atm = millis + 2000
    end
  end
  return res
end

#/// コマンドを送信する
#/// 最後に受信したデータが返ります
def command(cmddata)
  ackdata = "\xAA\x0E" + cmddata.bytes[1].chr + "\x00\x00\x00"
  clrbuf()
  txcmd(cmddata)
  rxchr = ""
  atm = millis + 1000
  syncStop = millis + 10000
  while(syncStop > millis) do
    if(Cam.available() > 0) then
      rxchr += Cam.read()
      if(rxchr.length >= 6)then
        if(rxchr[0,6] == ackdata)then
          return rxchr
          #break
        else
          clrbuf()
          rxchr = ""
          Usb.println "ACK Error Command ReTry"
          txcmd(cmddata)
          atm = millis + 1000
        end
      end
    end

    if(atm < millis)then
      clrbuf()
      rxchr = ""
      Usb.println "Time Over Command ReTry"
      txcmd(cmddata)
      atm = millis + 1000
    end
  end
  return ""
end

# 写真を撮ります
def takepicture
  #初期化します
  if(init() == false)then
    Usb.println "Initialization is failed."
    System.exit
  end
  Usb.println "Initialization is success."
      
  ##############
  ## 画像サイズをセットします
  #txdata = "\xAA\x01\x00\x07\x00\x01"   # QQQVGA 80x60 無効のようだ
  txdata = "\xAA\x01\x00\x07\x00\x03"   # QQVGA 160x120
  #txdata = "\xAA\x01\x00\x07\x00\x05"   # QVGA 320x240
  #txdata = "\xAA\x01\x00\x07\x00\x07"   # VGA 640x320
  if(command(txdata).length < 6)then
    Usb.println "Size seting is failed."
    System.exit
  end
  Usb.println "Size seting is success."

  #Usb.println "Wait 4sec...."
  #delay 4000

  #Set Packet Size
  txdata = "\xAA\x06\x08\x80\x00\x00"
  if(command(txdata).length < 6)then
    Usb.println "Set Packet Size is failed."
    System.exit
  end
  Usb.println "Set Packet Size is success."

  #Snapshot
  txdata = "\xAA\x05\x00\x00\x00\x00"
  if(command(txdata).length < 6)then
    Usb.println "Snapshot is failed."
    System.exit
  end
  Usb.println "Snapshot is success."

  #Get Picture
  txdata = "\xAA\x04\x01\x00\x00\x00"
  rxdata = command(txdata)
  if(rxdata.length < 6)then
    Usb.println "Get Picture is failed."
    System.exit
  end
  Usb.println "Get Picture is success."

  #Get Picture Size
  rxchr = ""
  if(rxdata.length > 6)then
    rxchr = rxdata[6, rxdata.length - 6]
  end
  atm = millis + 1000
  psize = 0
  while(atm > millis) do
    if(Cam.available() > 0) then
      rxchr += Cam.read()
      if(rxchr.length >= 6)then
        if(rxchr[0,3] == "\xAA\x0A\x01")then
          psize = (rxchr.bytes[3] | rxchr.bytes[4]<<8 | rxchr.bytes[5]<<16)
          break
        end
      end
    end
  end

  Usb.println "Picture Total Length is " + psize.to_s

  #Get Data
  pktCnt = (psize / (128 - 6)).floor
  if((psize % (128 - 6)) != 0)then
    pktCnt += 1
  end
  txdata = "\xAA\x0e\x00\x00\x00\x00"

  Usb.print "pktCnt= "
  Usb.println pktCnt.to_s

  #0x0000から3バイトのデータを読み込みます
  ans = System.pop(0x0000, 3)
  #インクリメントします
  num = ans.bytes[0] + ans.bytes[1] * 256 + ans.bytes[2] * 65536 + 1
  if(num > 99999)then
    num = 0
  end
  buf = (num & 0xff).chr + ((num >> 8) & 0xff).chr + ((num >> 16) & 0xff).chr
  System.push( 0x0000, buf, 3 )

  fname = "00000" + num.to_s
  fname = "IPN" + fname[fname.length-5..fname.length] + ".jpg"
  Usb.println "jpgファイル名: " + fname

  fn = SD.open(0, fname, 2) #新規オープン
  if(fn < 0)then
    Usb.println "File Open is failed."
    System.exit
  end

  for i in 0..(pktCnt - 1)
    Usb.println i.to_s
    txdata = txdata[0,4] + (i & 0xff).chr + ((i >> 8) & 0xff).chr

    Usb.println "Read Data"
    for retryCnt in 1..100 #100回リトライする
      delay 10
      Usb.println retryCnt.to_s
      clrbuf()
      rxchr = ""
      txcmd(txdata)
      atm = millis + 1000
      while(atm > millis) do
        if(Cam.available() > 0) then
          rxchr += Cam.read()
          if(rxchr.length >= 128)then
            break
          end
        end
      end

      Usb.println rxchr.length.to_s
      sum = 0
      for k in 0..(rxchr.length - 3)
        if((k % 64) == 0)then
          GC.start  #GCしないとインクリメンタルGCが追い付かない
        end
        sum += rxchr.bytes[k]
        sum = sum & 0xff
      end

      Usb.println "sum= " + sum.to_s + " check= " + (rxchr.bytes[rxchr.length - 2]).to_s
      if(sum == rxchr.bytes[rxchr.length - 2])then
        break
      end
      Usb.println "Read ReTry."
    end
    Usb.println "SD writing."
    SD.write(fn, rxchr[4, rxchr.length - 6], rxchr.length - 6)
  end

  txdata = txdata[0,4] + "\xf0\xf0"
  txcmd(txdata)

  SD.close(fn)
end

def getTimeBody() 
  tm = Rtc.getTime
  year = tm[0].to_s
  month = "00" + tm[1].to_s
  day = "00" + tm[2].to_s
  hour = "00" + tm[3].to_s
  min = "00" + tm[4].to_s
  sec = "00" + tm[5].to_s
  body = year + "/"
  body += month[month.length-2..month.length] + "/"
  body += day[day.length-2..day.length] + " "
  body += hour[hour.length-2..hour.length] + ":"
  body += min[min.length-2..min.length] + ":"
  body += sec[sec.length-2..sec.length]
  return body
end  

############## とりあえず、プログラムスタートします。
##電源ON時に、一度、写真を撮る
takepicture()

Usb.println "WiFi Ready"
Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect

Usb.println "WiFi Mode Setting"
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

Usb.println "WiFi access point"
#Usb.println WiFi.softAP "GR-CITRUS","37003700",2,3
Usb.println WiFi.softAP "Ippin CAMERA 192.168.4.1","37003700",2,3

Usb.println "WiFi dhcp enable"
Usb.println WiFi.dhcp 0,1

Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "WiFi multiConnect Set"
Usb.println WiFi.multiConnect 1

Usb.println WiFi.httpServer(-1).to_s
delay 100

body0 = '<html><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8">'
body0 += '<title>Ippin Camera</title></head>'
body0 += '<body><form method="get">'
body0 += '<h1>TIME: '
body2 = '<input type="text" name="rtc" size="16"> <input type="submit" value="TIME SET">'
body2 += '</h1></form><form method="get">'
body2 += "<h2>Take a Picture<br><button type='submit' name='shutter' value='1'>Take a Picture</button><br>"
body2 += '</h2></form><form method="get">'
body2 += "<br><br><br><button type='submit' name='exit' value='1'>System EXIT</button><br>"
body2 += '</form></body></html>'
body2 += "\r\n\r\n"

header0 = "HTTP/1.1 200 OK\r\n"
header0 += "Server: GR-CITRUS\r\n"
header0 += "Content-Type: text/html\r\n"
#header0 += "Date: Sun, 22 Oct 2017 12:00:00 GMT\r\n"
header0 += "Connection: close\r\n"
#header += "Content-Length: " + bodybtn.length.to_s + "\r\n\r\n"

Usb.println WiFi.httpServer(80).to_s

#header.txtを読み込む
def headview
  fn = SD.open(0, 'header.txt', 0)
  if(fn < 0)then
    Usb.println "SD open error"
  else
    size = SD.size(fn)
    for i in 0..size
      Usb.print SD.read(fn).chr
    end
    SD.close(fn)
  end
end

ledi = 0
while(true)do
  res, cnum = WiFi.httpServer
  #Usb.println res.to_s
  if(res == "/")then
    Usb.println res

    body = body0
    body += getTimeBody + "<br>"
    body += body2
    header = header0
    header += "Content-Length: " + body.length.to_s + "\r\n\r\n"

    WiFi.send(cnum, header)
    WiFi.send(cnum, body)

    Usb.println body
    headview()
  elsif(res.to_s[0,6] == "/?rtc=")
    Usb.println res
    if(res.to_s.length == 20)then
      year = res.to_s[6,4].to_i
      month = res.to_s[10,2].to_i
      day = res.to_s[12,2].to_i
      hour = res.to_s[14,2].to_i
      min = res.to_s[16,2].to_i
      sec = res.to_s[18,2].to_i
      
      Rtc.setTime([year,month,day,hour,min,sec])
    end
    body = body0
    body += getTimeBody + "<br>"
    body += body2
    header = header0
    header += "Content-Length: " + body.length.to_s + "\r\n\r\n"
    WiFi.send(cnum, header)
    WiFi.send(cnum, body)
    Usb.println body
    headview()
  elsif(res == "/?shutter=1")
    Usb.println res

    takepicture()

    body = body0
    body += getTimeBody + "<br>"
    body += body2
    header = header0
    header += "Content-Length: " + body.length.to_s + "\r\n\r\n"
    WiFi.send(cnum, header)
    WiFi.send(cnum, body)
    Usb.println body
    headview()
  elsif(res == "/?exit=1")
    Usb.println res

    body = body0
    body += getTimeBody + "<br>"
    body += body2
    header = header0
    header += "Content-Length: " + body.length.to_s + "\r\n\r\n"
    WiFi.send(cnum, header)
    WiFi.send(cnum, body)
    Usb.println body
    headview()
    break
  elsif(res == "0,CLOSED\r\n")
    Usb.println res

    headview()
  elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
    Usb.println "Else(*,:" + res
  
    Usb.println body
    headview()
  elsif(res != 0)
    Usb.println "Else:" + res.to_s
      
    body = body0
    body += getTimeBody + "<br>"
    body += body2
    header = header0
    header += "Content-Length: " + body.length.to_s + "\r\n\r\n"
    WiFi.send(cnum, header)
    WiFi.send(cnum, body)
    Usb.println body
    headview()
  end
  delay 0
  ledi += 1
  if((ledi % 500) == 0)then
    led
    ledi = 0
  end
end

#WiFi.send 0, "OK"
WiFi.httpServer(-1)
Usb.println WiFi.disconnect