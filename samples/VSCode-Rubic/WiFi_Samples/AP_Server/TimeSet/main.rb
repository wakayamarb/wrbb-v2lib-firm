#!mruby
#GR-CITRUS Version 2.35

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,OUTPUT)
digitalWrite(5,LOW)   # LOW:Disable
delay 500
digitalWrite(5,HIGH)   # LOW:Disable
delay 500

Usb = Serial.new(0,115200)

if(!System.use?('WiFi'))then
  Usb.println "WiFi Card can't use."
  System.exit() 
end

tm = Rtc.getTime
if(tm[0] < 2010)then
  Usb.println 'RTC Initialized'
  Rtc.init
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

Usb.println "WiFi Ready"
Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect

Usb.println "WiFi Mode Setting"
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

Usb.println "WiFi access point"
#Usb.println WiFi.softAP "GR-CITRUS","37003700",2,3
Usb.println WiFi.softAP "192.168.4.1","37003700",2,3

Usb.println "WiFi dhcp enable"
Usb.println WiFi.dhcp 0,1

Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "WiFi multiConnect Set"
Usb.println WiFi.multiConnect 1

Usb.println WiFi.httpServer(-1).to_s
delay 100

body0 = '<html><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8">'
body0 += '<title>RTC Setting</title></head>'
body0 += '<body><form method="get" >'
body0 += '<h1 align="center" size="16" style="width: 70%; padding: 0px;font-size:30px;">TIME SET<br>'

body2 = '<br><input type="text" name="rtc" size="16" style="width: 70%; padding: 0px;font-size:30px;"><br>'
body2 += '<input type="submit" value="SET" style="width: 70%; padding: 0px;font-size:30px;"><br>'
body2 += "</h1></form>"

body2 += '<form method="get">'
body2 += '<br><button type="submit" name="exit" value="1" style="width: 70%; padding: 0px;font-size:30px;">EXIT</button><br>'
body2 += "</form>"
body2 += "</body></html>"
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
end

#WiFi.send 0, "OK"
WiFi.httpServer(-1)
Usb.println WiFi.disconnect