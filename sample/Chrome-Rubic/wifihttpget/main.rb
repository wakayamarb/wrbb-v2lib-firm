#!mruby
#GR-CITRUS Version 2.15

#ESP8266を一度停止させる(リセットと同じ)
pinMode(5,1)
digitalWrite(5,0)   # LOW:Disable
delay 500
digitalWrite(5,1)   # LOW:Disable

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

#heds=["User-Agent: curl"]

for value in 1..10
  Usb.println WiFi.httpGet("192.168.1.58:3000/?value1=" + value.to_s + "&value2=" + (value*value).to_s).to_s
end
 
#Usb.println "WiFi disconnect"
#Usb.println WiFi.disconnect
