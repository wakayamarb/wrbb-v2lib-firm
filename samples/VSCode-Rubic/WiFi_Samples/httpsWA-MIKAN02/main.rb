#!mruby
#GR-CITRUS Version 2.46
#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # HIGH:Enable
delay 500

if(!System.use?("WiFi"))
  puts "WA-MIKAN can't use."
  System.exit
end
if(!System.use?("SD"))
  puts "SD can't use."
  System.exit
end

Usb = Serial.new(0,115200)

def SetRTC()
  tm1 = Rtc.getTime
  delay 1100
  tm2 = Rtc.getTime
  if(tm1[5] == tm2[5] || tm1[0] < 2010)then
    tm = WiFi.ntp("ntp.jst.mfeed.ad.jp", 9)
    Rtc.init
    if(!tm.instance_of?(Array))
      Rtc.setTime([2000,1,1,0,0,0])
    end
    Rtc.init
    Rtc.setTime(tm)
  end
  Rtc.getTime
end

def Filedisp(filename)
  puts "\r\n====== SD File Open:" + filename + " ======\r\n"
  #保存したhtmlファイルの読込み
  fn = SD.open(0,filename)
  if(fn < 0)then
    puts "File Open Error."
    return
  end

  while(true)do
    c = SD.read(fn)
    if(c < 0)then
      break
    end
    if(c == 0x0a)then
      Usb.println()
    else
      Usb.print(c.chr)
    end
  end
  SD.close(fn)
end

#puts WiFi.version
puts WiFi.connect("TAROSAY","37000")
#puts WiFi.ipconfig

puts SetRTC()

puts WiFi.setMode 1
#puts WiFi.multiConnect 1

heds=[
 "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:56.0) Gecko/20100101 Firefox/56.0",
 "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*,q=0.8",
 "Accept-Language: ja,en-US;q=0.7,en;q=0.3",
 "Accept-Encoding: gzip, deflate",
 "Connection: keepalive",
 "Upgrade-Insecure-Requests: 1"
]

if(SD.exists('index.htm'))
  SD.remove('index.htm')
  puts 'index.htmを削除しました'
end

#https 通信
#puts WiFi.httpsGetSD("index.htm",'twitter.com/?lang=ja')  #-> HTTP/1.1 200 OK
#puts WiFi.httpsGetSD("index.htm",'www.instagram.com') #-> HTTP/1.1 200 OK タイミングでつながらないときもある
#puts WiFi.httpsGetSD("index.htm",'www.facebook.com',heds)  #-> HTTP/1.1 200 OK

#http 通信 (https通信で接続できたもの)
#puts WiFi.httpGetSD("index.htm",'twitter.com/?lang=ja') #-> HTTP/1.1 301 Moved Permanently
#puts WiFi.httpGetSD("index.htm",'www.instagram.com')    #-> HTTP/1.1 301 Moved Permanently
#puts WiFi.httpGetSD("index.htm",'www.facebook.com')  #-> HTTP/1.1 302 Found

#http 通信 (https通信で接続できないもの)
#puts WiFi.httpGetSD("index.htm",'google.co.jp') #-> HTTP/1.1 301 Moved Permanently
#puts WiFi.httpGetSD("index.htm",'yahoo.co.jp') #-> HTTP/1.1 301 Moved Permanently
#puts WiFi.httpGetSD("index.htm",'qiita.com')  #-> HTTP/1.1 301 Moved Permanently
puts WiFi.httpGetSD("index.htm",'tarosay.github.io/httpssiteTest/')  #-> HTTP/1.1 301 Moved Permanently
#puts WiFi.httpGetSD("index.htm",'connpass.com/dashboard') #-> HTTP/1.1 301 Moved Permanently

#puts WiFi.at "CIPSSLSIZE=4096"
#puts WiFi.at 'CIPSTART=4,"SSL","badssl.com",443'
puts WiFi.disconnect
Filedisp("index.HTM")
