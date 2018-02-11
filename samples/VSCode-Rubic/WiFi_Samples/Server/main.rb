#!mruby
#GR-CITRUS Version 2.16

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable
delay 500

Usb = Serial.new(0,115200)

if( System.useWiFi() == 0)then
    Usb.println "WiFi Card can't use."
   System.exit() 
end
Usb.println "WiFi Ready"

Usb.println "WiFi disconnect"
Usb.println WiFi.disconnect

Usb.println "WiFi Mode Setting"
Usb.println WiFi.setMode 3  #Station-Mode & SoftAPI-Mode

Usb.println "WiFi connecting"
#Usb.println WiFi.connect("TAROSAY","37000")
Usb.println WiFi.connect("000740DE0D79","")

Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "WiFi multiConnect Set"
Usb.println WiFi.multiConnect 1

Usb.println WiFi.httpServer(-1).to_s
delay 100

Usb.println WiFi.httpServer(80).to_s

20.times do
    res = WiFi.httpServer
    Usb.println res.to_s
    if(res != 0)then
        break
    end
    delay 1000
end

WiFi.send 0, "OK"
WiFi.httpServer(-1)

#Usb.println "WiFi disconnect"
#Usb.println WiFi.disconnect
