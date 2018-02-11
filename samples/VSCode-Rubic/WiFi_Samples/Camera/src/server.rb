#!mruby
# GR-CITRUS Version 2.34
#ESP8266を一度停止させる(リセットと同じ)
pinMode(5, OUTPUT)
digitalWrite(5, LOW)   # LOW:Disable
delay 500
digitalWrite(5, HIGH)   # LOW:Disable
delay 500

Usb = Serial.new(0,115200)

if(!System.use?('WiFi'))then
  Usb.println "WiFi can't use."
  System.exit() 
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

bodybtn = '<html><head><meta http-equiv="Content-Type" content="text/html; charset=utf-8">'
bodybtn += '<title>LED Switch</title></head>'
bodybtn += '<body><form method="get" >'
bodybtn += '<h1 align="center">LED Switch<br>'
bodybtn += "<button type='submit' name='led' value='1'>LED ON</button><br>"
bodybtn += "<button type='submit' name='led' value='0'>LED OFF</button><br>"
bodybtn += "<button type='submit' name='exit' value='1'>EXIT</button><br>"
bodybtn += "</h1></form></body></html>"
bodybtn += "\r\n\r\n"

headerbtn = "HTTP/1.1 200 OK\r\n"
headerbtn += "Server: GR-CITRUS\r\n"
headerbtn += "Content-Type: text/html\r\n"
headerbtn += "Date: Sun, 22 Oct 2017 12:00:00 GMT\r\n"
headerbtn += "Connection: close\r\n"
headerbtn += "Content-Length: " + bodybtn.length.to_s + "\r\n\r\n"

Usb.println WiFi.httpServer(80).to_s

while(true)do
  res = WiFi.httpServer
  #Usb.println res.to_s
  if(res == "/")then
    Usb.println res
      
    WiFi.send(0, headerbtn)
    WiFi.send(0, bodybtn)
  elsif(res == "/?led=0")
    Usb.println res
    led 0
    WiFi.send(0, headerbtn)
    WiFi.send(0, bodybtn)
  elsif(res == "/?led=1")
    Usb.println res
    led 1
    WiFi.send(0, headerbtn)
    WiFi.send(0, bodybtn)
  elsif(res == "/?exit=1")
    Usb.println res

    WiFi.send(0, headerbtn)
    WiFi.send(0, bodybtn)
    break
  elsif(res == "0,CLOSED\r\n")
    Usb.println res

  elsif(res.to_s.length > 2 && ((res.bytes[0].to_s + res.bytes[1].to_s  == "0,") || (res.bytes[0].to_s + res.bytes[1].to_s  == "1,")))
    Usb.println "Else(*,:" + res
  
  elsif(res != 0)
    Usb.println "Else:" + res.to_s
      
    WiFi.send(0, headerbtn)
    WiFi.send(0, bodybtn)
  end
  delay 0
end

#WiFi.send 0, "OK"
WiFi.httpServer(-1)
Usb.println WiFi.disconnect