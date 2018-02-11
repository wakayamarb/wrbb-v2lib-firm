#!mruby
#GR-CITRUS Version 2.34

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

Usb.println "WiFi access point"
Usb.println WiFi.softAP "RubiRobo","37003700",2,3

Usb.println "WiFi dhcp enable"
Usb.println WiFi.dhcp 0,1

Usb.println "WiFi ipconfig"
Usb.println WiFi.ipconfig

Usb.println "WiFi multiConnect Set"
Usb.println WiFi.multiConnect 1

Usb.println WiFi.httpServer(-1).to_s
delay 100

Usb.println WiFi.httpServer(80).to_s

120.times do
    res = WiFi.httpServer
    Usb.println res.to_s
    if(res != 0)then
        break
    end
    delay 1000
    Usb.println WiFi.connetedIP
end

WiFi.send 0, "OK"
WiFi.httpServer(-1)

#Usb.println "WiFi disconnect"
#Usb.println WiFi.disconnect
